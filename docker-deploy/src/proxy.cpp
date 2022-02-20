#include "proxy.h"

#include <assert.h>

#include "exception.h"
#include "function.h"
#include "logger.h"
#include "parserRequest.h"
#include "parserResponse.h"

using namespace std;

#define MAX_LENGTH 65536

ofstream logfile("proxy.log");
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
cache my_cache;

/*
    The main process of proxy. Create a listen socket to accpet connection from client.
    It will create new thread for each new request.
*/
void proxy::run() {
  int server_fd;
  try {
    server_fd = createServerSocket(port);
  }
  catch (const std::exception & e) {
    std::cerr << e.what() << '\n';
    return;
  }

  // daemon
  int clientId = 0;
  while (1) {
    string clientIp;
    int client_fd;

    if (clientId % 200 == 0 && clientId != 0) {
      pthread_t thread;
      pthread_create(&thread, NULL, cleanCache, NULL);
    }

    try {
      client_fd = serverAcceptConnection(server_fd, clientIp);
    }
    catch (const std::exception & e) {
      std::cerr << clientId << ":" << e.what() << '\n';
      continue;
    }

    clientInfo * info = new clientInfo(
        clientId, clientIp, client_fd);  //这里需要加锁吗？ 主线程delete会让其他线程崩溃
    pthread_t thread;
    pthread_create(&thread, NULL, handleRequest, info);
    clientId++;
  }
}

/*
    handle request from client. Parsing the request and then handle it based on method.
*/
void * proxy::handleRequest(void * info) {
  clientInfo * client_info = (clientInfo *)info;

  // receive first request from client
  vector<char> buffer(MAX_LENGTH, 0);
  int len = recv(client_info->client_fd,
                 &(buffer.data()[0]),
                 MAX_LENGTH,
                 0);  // len 是recv实际的读取字节数
  if (len <= 0) {
    logger::InvalidRequest(to_string(client_info->Id));
    resourceClean(-1, client_info);
    return nullptr;
  }

  //parse the request,check its format
  //这里用string来接受client发起的第一个请求。因为client请求不会太长，所以string能装下。
  string request(buffer.data(), len);
  parserRequest p;
  try {
    p.parse(request);
  }
  catch (const std::exception & e) {
    std::cerr << client_info->Id << ":" << e.what() << '\n';
    logger::PrintException(client_info->Id, e.what());
    resourceClean(-1, client_info);
    return nullptr;
  }

  // log the new request
  logger::printNewRequest(client_info, p.requestline);

  // connect to server
  int server_fd;
  try {
    server_fd = clientRequestConnection(p.host, p.port);
  }
  catch (const std::exception & e) {
    std::cerr << client_info->Id << ":" << e.what() << '\n';
    resourceClean(-1, client_info);
    return nullptr;
  }

  // handle specific method
  if (p.method == "CONNECT") {
    logger::displayRequest(client_info, p.requestline, p.host);
    try {
      handle_CONNECT(client_info->client_fd, server_fd);
    }
    catch (const std::exception & e) {
      std::cerr << client_info->Id << ":" << e.what() << '\n';
      logger::PrintException(client_info->Id, e.what());
      resourceClean(server_fd, client_info);
      return nullptr;
    }
    logger::printTunnelClosed(client_info);
  }
  else if (p.method == "GET") {
    logger::displayRequest(client_info, p.requestline, p.host);
    try {
      handle_GET(client_info, server_fd, p);
    }
    catch (const std::exception & e) {
      std::cerr << client_info->Id << ":" << e.what() << '\n';
      logger::PrintException(client_info->Id, e.what());
      resourceClean(server_fd, client_info);
      return nullptr;
    }
  }
  else if (p.method == "POST") {
    //cout<<"post报文如下:\n";
    //cout<<p.request<<endl;
    try {
      handle_POST(server_fd, client_info, request, p);
    }
    catch (const std::exception & e) {
      std::cerr << client_info->Id << ":" << e.what() << '\n';
      logger::PrintException(client_info->Id, e.what());
      resourceClean(server_fd, client_info);
      return nullptr;
    }

    logger::displayRequest(client_info, p.requestline, p.host);
  }

  resourceClean(server_fd, client_info);

  return nullptr;
}

/*
  handle CONNECT for client. build a transparent tunnel between server and client.
  Receive data and then forward it directly to the other side withour reformating. 
*/
void proxy::handle_CONNECT(int client_fd, int server_fd) {
  send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);  //connect successfully, response

  //listen to two sides, diretly foward the message
  fd_set read_fds;
  vector<int> all_fds = {client_fd, server_fd};
  int max_fd = *max_element(all_fds.begin(), all_fds.end());
  while (1) {
    FD_ZERO(&read_fds);
    FD_SET(all_fds[0], &read_fds);
    FD_SET(all_fds[1], &read_fds);

    int res = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
    if (res < 0) {
      throw MyException("Fail to select!\n");
    }

    for (int i = 0; i < 2; i++) {
      vector<char> buffer(MAX_LENGTH, 0);
      int messageLen = 0;
      if (FD_ISSET(all_fds[i], &read_fds)) {
        // recv from one side
        messageLen = recv(all_fds[i], &(buffer.data()[0]), MAX_LENGTH, 0);

        if (messageLen <= 0)  // one side close the connection
          return;

        // send to the other side
        messageLen = send(all_fds[1 - i], &(buffer.data()[0]), messageLen, 0);
        if (messageLen <= 0)  // one side close the connection
          return;
      }
    }
  }
}

/*
  handle GET for client. proxy send client request to server directly. Then receive response from server.
  response can be chuncked or content-length mode. proxy handle it in differernt way.
*/
void proxy::handle_GET(clientInfo * client_info,
                       int server_fd,
                       const parserRequest & request_p) {
  string request = request_p.request;
  string URI = request_p.URI;

  cachedResponse target;
  if (my_cache.findInCache(URI, target) == true) {  // find resource in cache
    // resource need to revalidate
    if (target.mustRevalidate == true) {
      // revalidate the cached response to check its validity
      if (revalidateCache(target, server_fd, request_p, client_info) == true) {
        useCachedResponse(client_info, target.response);
        return;
      }
      else {
        my_cache.deleteResponse(URI);
        getResponseFromServer(server_fd, request_p, client_info);
        return;
      }
    }

    // if targe is expired, erase it from the cache. Then Re-request response from the server
    if (target.isExpired(time(0)) == true) {
      my_cache.deleteResponse(URI);
      getResponseFromServer(server_fd, request_p, client_info);
    }
    else {  // no expired, use the cache
      useCachedResponse(client_info, target.response);
    }
  }
  else {  // resource is not found in cache
    getResponseFromServer(server_fd, request_p, client_info);
  }
}

/*
  proxy send request to server and get the response. proxy parse the response to decide
  whether cache the "200 OK" response. proxy send back the response to client.

  Parameter: 
  server_fd : server socket
  request_p : the parser of the request. (request may be updated compared with the origin client request)
  client_info : store the client information
*/
void proxy::getResponseFromServer(int server_fd,
                                  const parserRequest & request_p,
                                  clientInfo * client_info) {
  string request = request_p.request;

  // send client request diretly to server
  if (send(server_fd, request.c_str(), request.length(), 0) <= 0) {
    throw MyException("Fail to send GET request to server.\n");
  }

  // receive first response from server
  vector<char> buffer(MAX_LENGTH, 0);
  int len = recv(server_fd, &(buffer.data()[0]), MAX_LENGTH, 0);
  if (len <= 0) {
    throw MyException("Fail to receive response from server.\n");
  }

  // parse response
  string firstResponse(buffer.data(), len);
  parserResponse response_p;
  response_p.parse(firstResponse);

  //write info into logfile
  logger::printReceievedResponse(client_info, response_p.status_line, request_p.host);

  // chunked mode. proxy recv packets from server and then directly forward to the client. do not cache
  if (response_p.chunked == true) {
    // send the first packet
    if (send(client_info->client_fd, firstResponse.c_str(), firstResponse.length(), 0) <
        0) {
      throw MyException("Fail to send chunk to client.\n");
    }
    while (1) {
      int len = recv(server_fd, &(buffer.data()[0]), MAX_LENGTH, 0);
      if (len <= 0)  //already send all chunks
        break;
      if (send(client_info->client_fd, &(buffer.data()[0]), len, 0) < 0) {
        throw MyException("Fail to send chunk to client.\n");
      }
    }
    cout << client_info->Id << ":finish send all the chunks.\n";
  }
  else {  // not chunked
    //get the whole message and then send it to the client
    int contentLength = stoi(response_p.content_length);
    int headerSize = response_p.head_length;
    string wholeMessage;
    getWholeMessage(
        len, contentLength, headerSize, server_fd, firstResponse, wholeMessage);
    if (send(client_info->client_fd, wholeMessage.c_str(), wholeMessage.length(), 0) <
        0) {
      throw MyException("fail to send whole GET response to client.\n");
    }

    // try to cache the response
    my_cache.insertCache(request_p.URI, wholeMessage, response_p);
  }
}

/*
  get thw whole response message from sever. Keep receiving until messageLen > remainLen.
  save the whole message in string & wholeMessage.
*/
void proxy::getWholeMessage(int firstMessageLen,
                            int contentLength,
                            int headerSize,
                            int server_fd,
                            const string & firstMessage,
                            string & wholeMessage) {
  wholeMessage = firstMessage;
  int remainReceiveLen =
      contentLength -
      (firstMessageLen - headerSize);  //firstMessage smaller than header is very rare.
  int receiveLen = 0;                  //record receive message len;
  vector<char> buffer(MAX_LENGTH, 0);

  while (receiveLen < remainReceiveLen) {
    int len = recv(server_fd, &(buffer.data()[0]), MAX_LENGTH, 0);
    if (len <= 0)
      break;
    string temp(buffer.data(), len);  //can not use the whole buffer as a string.
    wholeMessage += temp;
    receiveLen += len;
  }
  return;
}

/*
  clean resource. close sockets and delete clientInfo.
*/
void proxy::resourceClean(int server_fd, clientInfo * info) {
  if (server_fd != -1)
    close(server_fd);

  close(info->client_fd);
  delete (info);
}

/*
  handle POST method. get the whole POST request from client and then 
  forward it directly to server. Then forward back the server's response
*/
void proxy::handle_POST(int server_fd,
                        clientInfo * client_info,
                        string & request,
                        const parserRequest & p) {
  unordered_map<string, string> list = p.list;
  int content_length = stoi(list["Content-Length"]);
  int headerSize = p.head_length;

  //get whole POST request and send it to server
  string wholeMessage;
  getWholeMessage(
      request.length(), content_length, headerSize, server_fd, request, wholeMessage);
  if (send(server_fd, wholeMessage.c_str(), wholeMessage.length(), 0) < 0) {
    throw MyException("fail to send POST message to server.\n");
  }

  //receive server's response and forward it to client directly
  vector<char> buffer(MAX_LENGTH, 0);
  int len = recv(server_fd, &(buffer.data()[0]), MAX_LENGTH, 0);
  if (len < 0) {
    throw MyException("Fail to receive POST's reponse from serever.\n");
  }
  if (send(client_info->client_fd, buffer.data(), len, 0) < 0) {
    throw MyException("fail to send POST message's response to client.\n");
  }
}

/*
  send the cached Response to the client.
*/
void proxy::useCachedResponse(clientInfo * client_info, const string & response) {
  if (send(client_info->client_fd, response.c_str(), response.length(), 0) < 0) {
    throw MyException("Fail to send cached response to server.\n");
  }
}

/*
  send request to server to revalidate the cached response.
  attach to E-tag segment and last modified segment to the request.
  if server response 304 unmodified return true,  else return false.
*/
bool proxy::revalidateCache(const cachedResponse & target,
                            int server_fd,
                            const parserRequest & request_p,
                            clientInfo * client_info) {
  if (target.E_tag == "" && target.last_modified == "")
    return true;

  // modify origin request and add segment to the end of header
  string new_request = request_p.request;
  if (target.E_tag != "") {
    string add_etag = "If-None-Match: " + target.E_tag + "\r\n";
    new_request.insert(request_p.head_length - 2, add_etag.c_str());
  }
  else if (target.last_modified != "") {
    string add_modified = "If-Modified-Since: " + target.last_modified + "\r\n";
    new_request.insert(request_p.head_length - 2, add_modified.c_str());
  }

  // send revalidate request to server
  if (send(server_fd, new_request.c_str(), new_request.length(), 0) < 0) {
    throw MyException("fail to send revalidate request to server.\n");
  }

  // recv result from server
  vector<char> buffer(MAX_LENGTH, 0);
  int len = recv(server_fd, &(buffer.data()[0]), MAX_LENGTH, 0);
  if (len < 0) {
    cerr << client_info->Id << ": fail to recv revalidaion result from server.\n";
    return false;
  }

  // check result
  string res(buffer.data(), len);
  if (res.find("304") != std::string::npos) {
    return true;
  }
  return false;
}


/*
  clean the Cache, delete expired cached responsed
*/
void* proxy::cleanCache(void* ptr){
  my_cache.cleanCache();
  return nullptr;
}