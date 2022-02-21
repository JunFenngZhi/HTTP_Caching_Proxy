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

  static void printReceievedResponse(clientInfo * client_info,
                                     string status_line,
                                     string host) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": Received \"" << status_line << "\" from " << host
            << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  static void printNotInCache(clientInfo * client_info) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": not in cache" << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  static void printRequireValidation(clientInfo * client_info) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": in cache, requires validation" << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  //need to complete
  static void printExpiredCache(clientInfo * client_info, cachedResponse cp) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": in cache, but expired at" << cp.storedTime
            << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  static void printCacheValid(clientInfo * client_info) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": in cache, valid" << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  static void printNotCacheableChunked(clientInfo * client_info) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": not cacheable because chunked" << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  static void printResponding(clientInfo * client_info, string status_line) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": Responding \"" << status_line << "\"" << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  static void printNoteCacheControl(clientInfo * client_info, parserResponse response) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": NOTE Cache-Control \""
            << response.list["Cache-Control"] << "\"" << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  static void printNoteETag(clientInfo * client_info, parserResponse response) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": NOTE ETag \"" << response.list["ETag"] << "\""
            << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  static void printCachedNeedRevalidate(clientInfo * client_info) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": cached, but requires re-validation" << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  static void printCachedExpires(clientInfo * client_info, parserResponse response) {
    pthread_mutex_lock(&mutex);
    logfile << client_info->Id << ": cached, expires at \"" << response.list["Expires"]
            << "\"" << std::endl;
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