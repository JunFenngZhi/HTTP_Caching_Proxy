#include "cache.h"

pthread_rwlock_t rwlock;


/*
    insert response into cache. 
*/
void cache::insertCache(const string& key, const cachedResponse& response){

}


/*
    try to find the response in the cache. if find it, save it in target, return true.
    else return false.
*/
bool findCache(const string& key, cachedResponse & target){

}


/*
    clean the expired cachedResponse.
*/
void cleanCache(){

}