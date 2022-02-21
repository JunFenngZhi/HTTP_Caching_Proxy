#include "cache.h"

pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

/*
    insert response into cache. Response need to be "200 OK" without "no-store" attribute as well as max-age is not zero.
    check segment in parser to determine whether cache. Logging the result. 
*/
void cache::insertCache(const string & key,
                        const string & response,
                        const parserResponse & response_p) {
  if (response.find("HTTP/1.1 200 OK") != string::npos && response_p.no_store == false &&
      response_p.max_age != 0) {
    cachedResponse c(response_p.max_age,
                     response,
                     response_p.E_tag,
                     response_p.LastModified,
                     response_p.revalidate);

    //change from rdlock to wrlock
    pthread_rwlock_wrlock(&rwlock);
    list[key] = c;
    pthread_rwlock_unlock(&rwlock);
  }
}

/*
    try to find the response in the cache. if find it, save it in target, return true.
    else return false.
*/
bool cache::findInCache(const string & key, cachedResponse & target) {
  pthread_rwlock_rdlock(&rwlock);
  if (list.find(key) == list.end()) {  // not find
    pthread_rwlock_unlock(&rwlock);
    return false;
  }
  target = list[key];  // 这里会触发赋值运算符，可能使用指针返回地址更好。
  pthread_rwlock_unlock(&rwlock);
  return true;
}

/*
    clean the expired cachedResponse.
*/
void cache::cleanCache() {
  pthread_rwlock_wrlock(&rwlock);
  for (auto it = list.begin(); it != list.end();) {
    if (it->second.isExpired(time(0)) == true)
      it = list.erase(it);
    else
      ++it;
  }
  pthread_rwlock_unlock(&rwlock);
}

/*
    delete specific reponse in cache based on key.
*/
void cache::deleteResponse(const string & key) {
  pthread_rwlock_wrlock(&rwlock);
  list.erase(key);
  pthread_rwlock_unlock(&rwlock);
}