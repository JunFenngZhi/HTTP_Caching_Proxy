#include "proxy.h"
#include "clientInfo.h"
#include "function.h"
#include "logger.h"
#include "parserRequest.h"
#include "exception.h"

using namespace std;

#define MAX_LENGTH 65536

ofstream logfile("proxy.log");

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*
    The main process of proxy
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

    clientInfo * info = new clientInfo(clientId, clientIp, client_fd);  //这里需要加锁吗？ 需要delete吗？
    pthread_t thread;
    pthread_create(&thread, NULL, handleRequest, info);
    clientId++;
  }
}

/*
    handle request from client
*/
void * proxy::handleRequest(void * info) {
  clientInfo * client_info = (clientInfo *)info;
  vector<char> messageContainer(MAX_LENGTH, 0);

  //cout<<"enter new thread\n";

  // receive first request from client
  int len = recv(client_info->client_fd,
                 &(messageContainer.data()[0]),
                 MAX_LENGTH,
                 0);  // len 是recv实际的读取字节数
  if (len <= 0) {
    logger::InvalidRequestFormat(to_string(client_info->Id));
    return nullptr;
  }

  //cout<<"receive request\n";

  //clean messageContainer and then parse the request,check its format
  string request = messageContainer.data();
  messageContainer.clear();  //这里需不需要清空 ??
  parserRequest p;
  try {
    p.parse(request);
  }
  catch (const std::exception & e) {
    std::cerr << e.what() << '\n';
    logger::PrintException(e.what());
    return nullptr;
  }
  
  //p.printResult();

  // log the new request
  logger::printNewRequest(client_info, p.requestline);

  // connect to server
  int server_fd;
  try {
    server_fd = clientRequestConnection(p.host, p.port);
  }
  catch (const std::exception & e) {
    std::cerr << e.what() << '\n';
    return nullptr;
  }

  // handle specific method  //(logging 还没写)
  if (p.method == "CONNECT") {
    cout << "This a CONNECT method\n";
    try {
      handle_CONNECT(client_info->client_fd,server_fd);
    }
    catch (const std::exception & e) {
      std::cerr << e.what() << '\n';
      return nullptr;
    }
    cout<<"tunnel closed\n";
  }
  else if (p.method == "GET") {
    cout << "This a GET method\n";
  }
  else if (p.method == "POST") {
    cout << "This a POSt method\n";
  }
  close(server_fd);
  close(client_info->client_fd);


  return nullptr;
}

/*
  handle CONNECT for client. build a transparent tunnel between server and client.
*/
void proxy::handle_CONNECT(int client_fd, int server_fd) {
  send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);  //connect successfully, response

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