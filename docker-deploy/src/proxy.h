#ifndef _PROXY_H
#define _PROXY_H

#include <pthread.h>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

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
 static void* handleRequest(void* info);
 static void handle_CONNECT(int client_fd, int server_fd);

};

#endif
