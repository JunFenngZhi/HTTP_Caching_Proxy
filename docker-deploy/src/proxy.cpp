#include "proxy.h"
#include "clientInfo.h"
#include "function.h"
#include "logger.h"
#include "parserRequest.h"
#include "exception.h"
#include "assert.h"
#include "parserResponse.h"

using namespace std;

#define MAX_LENGTH 65536

ofstream logfile("proxy.log");

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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

    try {
      client_fd = serverAcceptConnection(server_fd, clientIp);
    }
    catch (const std::exception & e) {
      std::cerr << e.what() << '\n';
      continue;
    }

    clientInfo * info = new clientInfo(clientId, clientIp, client_fd);  //这里需要加锁吗？ 主线程delete会让其他线程崩溃
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
  vector<char> messageContainer(MAX_LENGTH, 0);
  int len = recv(client_info->client_fd,
                 &(messageContainer.data()[0]),
                 MAX_LENGTH,
                 0);  // len 是recv实际的读取字节数
  if (len <= 0) {
    logger::InvalidRequest(to_string(client_info->Id));
    close(client_info->client_fd);
    return nullptr;
  }
  
  //clean messageContainer and then parse the request,check its format
  //这里用string来接受client发起的第一个请求。因为client请求不会太长，所以string能装下。
  string request = messageContainer.data();  
  assert(len == request.length());
  parserRequest p;
  try {
    p.parse(request);
  }catch (const std::exception & e) {
    std::cerr << e.what() << '\n';
    logger::PrintException(client_info->Id, e.what());
    close(client_info->client_fd);
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
    std::cerr << e.what() << '\n';
    close(client_info->client_fd);
    return nullptr;
  }

  // handle specific method  
  if (p.method == "CONNECT") {
    logger::displayRequest(client_info, p.requestline, p.host);
    try {
      handle_CONNECT(client_info->client_fd,server_fd);
    }
    catch (const std::exception & e) {
      std::cerr << e.what() << '\n';
      return nullptr;
    }
    logger::printTunnelClosed(client_info);
  }
  else if (p.method == "GET") {
    logger::displayRequest(client_info, p.requestline, p.host);
    try{
      handle_GET(client_info, server_fd, request);
    }catch(const std::exception& e){
      std::cerr << e.what() << '\n';
      logger::PrintException(client_info->Id, e.what());
    }
    
    
    
  }
  else if (p.method == "POST") {
    logger::displayRequest(client_info, p.requestline, p.host);
  }
  
  close(server_fd);
  close(client_info->client_fd);

  delete info;
  
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
  handle GET for client.
*/
void proxy::handle_GET(clientInfo* client_info, int server_fd, const string& request) {
  // send client request diretly to server
  if(send(server_fd, request.c_str(), request.length(), 0) <= 0){
    throw MyException("Fail to send GET request to server.\n");
  }

  // receive first response from server
  vector<char> buffer(MAX_LENGTH, 0);
  int len = recv(server_fd,&(buffer.data()[0]),MAX_LENGTH,0);
  if(len <= 0){
    throw MyException("Fail to receive response from server.\n");
  }

  // parse response
  string firstResponse(buffer.data());
  parserResponse p;
  p.parse(firstResponse);

  // logging 待补充
  if(p.chunked == true){  
    // chunked mode. proxy recv packets from server and then directly forward to the client
    send(server_fd, firstResponse.c_str(),firstResponse.length(),0);  // send the first packet
    while (1){
      int len = recv(server_fd,&(buffer.data()[0]),MAX_LENGTH,0);
      if(len <= 0)  //already send all chunks
        break;
      send(client_info->client_fd,&(buffer.data()[0]), len, 0);  
    }
    cout<<"finish send all the chunks.\n"; 
  }
  else{  // not chunked
    //get the whole message and then send it to the client
    int contentLength = stoi(p.content_length);
    int headerSize = p.head_length;
    string wholeMessage = getWholeMessage(len,contentLength,headerSize,server_fd,firstResponse);
    send(client_info->client_fd, wholeMessage.c_str(), wholeMessage.length(), 0);
  }


}