#ifndef _LOGGER_H
#define _LOGGER_H

#include <ctime>

#include "proxy.h"

using namespace std;

class logger {
 public:
  logger() {}
  ~logger() {}

 public:
  static void InvalidRequest(const string & clientID) {
    pthread_mutex_lock(&mutex);
    logfile << clientID << ": Invalid request. recv return negative or zero." << endl;
    pthread_mutex_unlock(&mutex);
  }
  static void PrintException(int id, const string & error) {
    pthread_mutex_lock(&mutex);
    logfile << to_string(id) << ":" << error << endl;
    pthread_mutex_unlock(&mutex);
  }
  static void printNewRequest(clientInfo * client_info, string requestLine) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": \"" << requestLine << "\" from " << client_info->Ip
            << " @ " << getTime().append("\0");
    pthread_mutex_unlock(&mutex);
  }
  static void displayRequest(clientInfo * client_info, string requestLine, string host) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": "
            << "Requesting \"" << requestLine << "\" from " << host << std::endl;
    pthread_mutex_unlock(&mutex);
  }
  static void printTunnelClosed(clientInfo * client_info) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": Tunnel closed" << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  static string getTime() {
    time_t currTime = time(0);
    struct tm * nowTime = gmtime(&currTime);
    nowTime->tm_hour -= 5;  //打印为美东时间
    const char * t = asctime(nowTime);
    return string(t);
  }
};

#endif