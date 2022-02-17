#ifndef _PROXY_H
#define _PROXY_H

#include <pthread.h>

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "clientInfo.h"

using namespace std;

extern ofstream logfile;
extern pthread_mutex_t mutex;

class proxy {
 private:
  string port;  //server port number

 public:
  proxy(const char * portNum) : port(portNum) {}
  ~proxy() {}
  void run();

 private:
  static void * handleRequest(void * info);
  static void handle_CONNECT(int client_fd, int server_fd);
  static void handle_GET(clientInfo * client_info, int server_fd, const string & request);
  static void getWholeMessage(int firstMessageLen,
                              int contentLength,
                              int headerSize,
                              int serverId,
                              const string & firstResponse,
                              string & wholeMessage);
  static void resourceClean(int server_fd, clientInfo * info);
};

#endif
