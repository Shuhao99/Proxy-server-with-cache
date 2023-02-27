#include "cache.hpp"
std::string getMaxage(const std::string &cacheControl) {
  size_t ageStart = cacheControl.find("max-age");
  if (ageStart == std::string::npos) {
    ageStart = cacheControl.find("s-maxage");
    if (ageStart == std::string::npos) {
      return std::string();
    } else {
      ageStart += 1;
    }
  }
  size_t uselessLength = 8; // length of "max-age="
  size_t ageEnd = cacheControl.find_first_of(" ,", ageStart);
  if (ageEnd == std::string::npos) {
    ageEnd = cacheControl.length();
  }
  std::string maxAge = cacheControl.substr(ageStart + uselessLength,
                                           ageEnd - ageStart - uselessLength);
  return maxAge;
}
bool isMaxageStale(const std::string &maxAgeString,
                   const std::map<std::string, std::string> &headers) {
  int maxAge = std::stoi(maxAgeString);

  std::map<std::string, std::string>::const_iterator dateIterator =
      headers.find("Date");
  if (dateIterator != headers.end()) {
    auto date = stringToDate(dateIterator->second);
    auto now = std::chrono::system_clock::from_time_t(getTimeNow());
    auto expires = date + std::chrono::seconds(maxAge);
    return expires <= now;
  }
  return true;
}
bool Cache::checkIfStale(const response &res) {
  std::map<std::string, std::string> headers = res.get_header();
  std::map<std::string, std::string>::const_iterator cacheControlIterator =
      headers.find("Cache-Control");
  if (cacheControlIterator == headers.end()) {
    std::map<std::string, std::string>::const_iterator expiresIterator =
        headers.find("Expires");
    if (expiresIterator != headers.end()) {
      auto expires = stringToDate(expiresIterator->second);
      auto now = std::chrono::system_clock::from_time_t(getTimeNow());
      return expires <= now;
    }
    return true;
  }
  std::string cacheControl = cacheControlIterator->second;
  std::string maxAgeString = getMaxage(cacheControl);
  if (maxAgeString.empty()) {
    // cacheControl == "no-cache" || cacheControl == "must-revalidate"
    // cacheControl == "max-age=0" || ... => need revalidate
    return true;
  }
  return isMaxageStale(maxAgeString, headers);
}

cacheResponse Cache::revalidate(const response &res) {
  cacheResponse cacheRes(true, false);
  cacheRes.res = res;

  std::map<std::string, std::string> headers = res.get_header();
  auto etagIterator = headers.find("ETag");
  if (etagIterator == headers.end()) {
    auto lastModifiedIterator = headers.find("Last-Modified");
    if (lastModifiedIterator == headers.end()) {
      return cacheResponse(false, false);
    } else {
      cacheRes.ifModifiedSince = lastModifiedIterator->second;
      return cacheRes;
    }
  }
  cacheRes.ifNoneMatch = etagIterator->second;
  return cacheRes;
}

void Cache::cacheLruDelete() {
  std::time_t min;
  std::string keyNeedRemove;
  for (auto iter = cache.begin(); iter != cache.end(); ++iter) {
    std::time_t time = (iter->second).second;
    if (iter == cache.begin()) {
      min = time;
      keyNeedRemove = iter->first;
    } else if (min > time) {
      min = time;
      keyNeedRemove = iter->first;
    }
  }
  if (!keyNeedRemove.empty()) {
    cache.erase(keyNeedRemove);
  }
}
void Cache::updateCache(const request &req, response res) {
  if (!checkCachable(res)) {
    return;
  }
  res.set_time();

  std::string key = getHashKey(req);
  auto resIterator = cache.find(key);
  if (resIterator == cache.end()) {
    cache.insert({key, std::make_pair(res, getTimeNow())});
    if (cache.size() >= maxCacheSize) {
      cacheLruDelete();
    }
  } else {
    resIterator->second = std::make_pair(res, getTimeNow());
  }
}

bool Cache::checkCachable(const response &res) {
  std::map<std::string, std::string> headers = res.get_header();
  auto iter = headers.find("Cache-Control");
  if (iter != headers.end()) {
    if (iter->second.find("no-store") != std::string::npos ||
        iter->second.find("private") != std::string::npos) {
      return false;
    }
  }
  return true;
}
// bool Cache::checkCachableStr(const response &res) {
//   std::map<std::string, std::string> headers = res.get_header();
//   auto iter = headers.find("Cache-Control");
//   if (iter != headers.end()) {
//     if (iter->second.find("no-store") != std::string::npos ||
//         iter->second.find("private") != std::string::npos) {
//       return false;
//     }
//   }
//   return true;
// }

cacheResponse Cache::getResponseFromCache(const request &req) {
  if (checkInCache(req)) {
    auto resIter = cache.find(getHashKey(req));
    response res = resIter->second.first;
    if (checkIfStale(res)) {
      return revalidate(res);
    } else {
      cacheResponse ans(true, true);
      ans.res = res;
      resIter->second.second = getTimeNow();
      return ans;
    }
  }
  return cacheResponse();
}

std::string Cache::getExpires(const request &req) {
  if (!checkInCache(req)) {
    return std::string();
  }
  auto iter = cache.find(getHashKey(req));
  response res = iter->second.first;
  auto headers = res.get_header();
  auto expireIter = headers.find("Expires");
  if (expireIter != headers.end()) {
    return expireIter->second;
  }
  auto cacheIter = headers.find("Cache-Control");
  if (cacheIter != headers.end()) {
    std::string cacheControl = cacheIter->second;
    std::string maxAgeString = getMaxage(cacheControl);
    if (!maxAgeString.empty()) {
      int maxAge = std::stoi(maxAgeString);
      std::map<std::string, std::string>::const_iterator dateIterator =
          headers.find("Date");
      if (dateIterator != headers.end()) {
        auto date = stringToDate(dateIterator->second);
        auto expires = date + std::chrono::seconds(maxAge);
        return dateToString(expires);
      }
    }
  }
  return std::string();
}
