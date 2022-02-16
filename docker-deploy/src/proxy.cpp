#include "proxy.h"

#include "clientInfo.h"
#include "function.h"
#include "logger.h"
#include "parserRequest.h"

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
    
    cout<<clientIp<<endl;

    clientInfo * info = new clientInfo(clientId, clientIp, client_fd);  //这里需要加锁吗？
    pthread_t thread;
    pthread_create(&thread, NULL, handleRequest, info);
    clientId++;
    delete info;
  }
}

/*
    handle request from client
*/
void * proxy::handleRequest(void * info) {
  clientInfo * client_info = (clientInfo *)info;
  vector<char> messageContainer(MAX_LENGTH,0);
  cout<<"enter new thread\n";

  // receive first request from client
  int len = recv(client_info->client_fd, &(messageContainer.data()[0]), MAX_LENGTH, 0);  // len 是recv实际的读取字节数
  if (len <= 0) {
    logger::InvalidRequestFormat(to_string(client_info->Id));
    return nullptr;
  }
  cout<<"receive request\n";
  //clean messageContainer and then parse the request,check its format
  string request = messageContainer.data();
  messageContainer.clear();  //这里需不需要清空 ??
  parserRequest p;
  try{
    p.parse(request); 
  }catch(const std::exception& e){
    std::cerr << e.what() << '\n';
    logger::PrintException(e.what());
    return nullptr;
  }
  p.printResult();

  // log the new request
  logger::printNewRequest(client_info,p.requestline);




  
  


  return nullptr;
}