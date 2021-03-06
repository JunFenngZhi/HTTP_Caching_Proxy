#ifndef _PROXY_H
#define _PROXY_H

#include <pthread.h>

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "cache.h"
#include "clientInfo.h"
#include "parserRequest.h"

using namespace std;

extern ofstream logfile;
extern pthread_mutex_t mutex;

class proxy {
 private:
  string port;  //proxy port number

 public:
  proxy(const char * portNum) : port(portNum) {}
  ~proxy() {}
  void run();

 private:
  static void * handleRequest(void * info);
  static void handle_CONNECT(clientInfo * client_info, int client_fd, int server_fd);
  static void handle_GET(clientInfo * client_info,
                         int server_fd,
                         const parserRequest & request_p);
  static void getWholeMessage(int firstMessageLen,
                              int contentLength,
                              int headerSize,
                              int server_fd,
                              const string & firstMessage,
                              string & wholeMessage);
  static void resourceClean(int server_fd, clientInfo * info);
  static void handle_POST(int server_fd,
                          clientInfo * client_info,
                          string & request,
                          const parserRequest & p);
  static void getResponseFromServer(int server_fd,
                                    const parserRequest & request_p,
                                    clientInfo * client_info);
  static void useCachedResponse(clientInfo * client_info, const string & response);
  static bool revalidateCache(const cachedResponse & target,
                              int server_fd,
                              const parserRequest & request_p,
                              clientInfo * client_info);
  static void * cleanCache(void * ptr);
};

#endif
