#ifndef _LOGGER_H
#define _LOGGER_H

#include "proxy.h"
#include <ctime>

using namespace std;

class logger {
 public:
  logger() {}
  ~logger() {}

 public:
  static void InvalidRequestFormat(const string & clientID) {
    pthread_mutex_lock(&mutex);
    logfile << clientID << ": Invalid request format" << endl;
    pthread_mutex_unlock(&mutex);
  }
  static void PrintException(const string & error) {
    pthread_mutex_lock(&mutex);
    logfile << error << endl;
    pthread_mutex_unlock(&mutex);
  }
  static void printNewRequest(clientInfo * client_info, string requestLine) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": \"" << requestLine << "\" from "
            << client_info->Ip << " @ " << getTime().append("\0");
    pthread_mutex_unlock(&mutex);
  }
  static string getTime() {
    time_t currTime = time(0);
    struct tm * nowTime = gmtime(&currTime);
    const char * t = asctime(nowTime);
    return string(t);
  }
};

#endif