#include "cache.h"

pthread_rwlock_t rwlock;


/*
    insert response into cache. Response need to be "200 OK" without "no-store" attribute.
    check segment in parser to determine whether cache. Logging the result. 
*/
void cache::insertCache(const string & key, const string & response, parserResponse & p){

}


/*
    try to find the response in the cache. if find it, save it in target, return true.
    else return false.
*/
bool findInCache(const string& key, cachedResponse & target){

}


/*
    clean the expired cachedResponse.
*/
void cleanCache(){

}

/*
    delete specific reponse in cache based on key.
*/
void deleteResponse(const string& key){

}