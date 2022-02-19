#ifndef _CACHE_H
#define _CACHE_H

#include <pthread.h>

#include <ctime>
#include <string>
#include <unordered_map>

#include "parserResponse.h"

using namespace std;

extern pthread_rwlock_t rwlock;

class cachedResponse {
 public:
  time_t storedTime;  // the time that response beging stored (local time)
  string response;
  string E_tag;
  string last_modified;
  int max_age;
  bool mustRevalidate;

  cachedResponse(int max_age,
                 const string & message,
                 const string & E_tag,
                 const string & last_modified,
                 bool mustRevalidate) :
      storedTime(time(0)),
      response(message.c_str()),
      E_tag(E_tag.c_str()),
      last_modified(last_modified.c_str()),
      mustRevalidate(mustRevalidate),
      max_age(max_age) {}

  cachedResponse() {}

  /*
    check whether cachedResponse is expired.
    return true if it is expired, else return false.
  */
  bool isExpired(time_t curTime) {
    if (max_age = -1)  // no expire time
      return false;
    if ((curTime - storedTime) > max_age) {
      return true;
    }
    return false;
  }
};

class cache {
 private:
  unordered_map<string, cachedResponse> list;

 public:
  cache() { /*TODO: initialize RW_mutex*/
  }
  ~cache() {}
  void insertCache(const string & key, const string & response, parserResponse & p);
  bool findInCache(const string & key, cachedResponse & target);
  void cleanCache();
  void deleteResponse(const string& key);
};

#endif
