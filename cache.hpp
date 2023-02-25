#ifndef CACHE_H
#define CACHE_H

#include "time_util.h"
#include "request.h"
#include "response.h"
#include <string>
#include <map>
#include <utility>
#define defaultMax 100
class cacheResponse {
public:
  bool isFind;
  bool isFresh;
  std::string ifNoneMatch; //""
  std::string ifModifiedSince; 
  response res;
  cacheResponse()
      : isFind(false), isFresh(false), ifNoneMatch(std::string()),
        ifModifiedSince(std::string()) {}
  cacheResponse(bool find, bool fresh)
      : isFind(find), isFresh(fresh), ifNoneMatch(std::string()),
        ifModifiedSince(std::string()) {}
};
class Cache {
private:
  std::map<std::string, std::pair<response, std::time_t>> cache;
  size_t maxCacheSize;

public:
  Cache() : maxCacheSize(defaultMax) {}
  Cache(size_t x) : maxCacheSize(x) {}
  std::string getHashKey(const request &req) { return req.get_msg(); }
  bool checkInCache(const request &req) {
    return cache.find(getHashKey(req)) != cache.end();
  }
  bool checkIfStale(const response &res);
  cacheResponse revalidate(const response &res);
  void updateCache(const request &req, response res);
  void cacheLruDelete();
  bool checkCachable(const response &res);
  
  cacheResponse getResponseFromCache(const request &req);
  std::string getExpires(const request &req);
};

#endif
