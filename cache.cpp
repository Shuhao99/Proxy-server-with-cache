#include "cache.hpp"
std::string getInfoFromControl(const std::string &cacheControl,
                               std::string info) {
  size_t infoStart = cacheControl.find(info);
  if (infoStart == std::string::npos) {
    return std::string();
  }
  size_t uselessLength = info.size() + 1;
  size_t infoEnd = cacheControl.find_first_of(" ,", infoStart);
  if (infoEnd == std::string::npos) {
    infoEnd = cacheControl.length();
  }
  std::string ans = cacheControl.substr(infoStart + uselessLength,
                                        infoEnd - infoStart - uselessLength);
  return ans;
}
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
bool checkReqStale(const std::map<std::string, std::string> &headers,
                   std::chrono::system_clock::time_point expires,
                   std::chrono::system_clock::time_point now,
                   const reqControl &reqCon) {

  std::string maxAgeStr = reqCon.maxAge;
  std::string maxStaleStr = reqCon.maxStale;
  std::string minFreshStr = reqCon.minFresh;

  if (!maxAgeStr.empty()) {
    int maxAge = std::stoi(maxAgeStr);
    auto dateIter = headers.find("Date");
    if (dateIter != headers.end()) {
      auto date = stringToDate(dateIter->second);
      auto reqExpires = date + std::chrono::seconds(maxAge);
      if (reqExpires < expires) {
        return reqExpires <= now;
      } else {
        return expires <= now;
      }
    }
  }
  if (!maxStaleStr.empty()) {
    int maxStale = std::stoi(maxStaleStr);
    auto reqExpires = expires + std::chrono::seconds(maxStale);
    if (reqExpires < expires) {
      return reqExpires <= now;
    } else {
      return expires <= now;
    }
  }
  if (!minFreshStr.empty()) {
    int minFresh = std::stoi(minFreshStr);
    auto reqExpires = expires - std::chrono::seconds(minFresh);
    if (reqExpires < expires) {
      return reqExpires <= now;
    } else {
      return expires <= now;
    }
  }
  return expires <= now;
}
bool isMaxageStale(const std::string &maxAgeString,
                   const std::map<std::string, std::string> &headers,
                   const reqControl &reqCon) {
  int maxAge = std::stoi(maxAgeString);

  std::map<std::string, std::string>::const_iterator dateIterator =
      headers.find("Date");
  if (dateIterator != headers.end()) {
    auto date = stringToDate(dateIterator->second);
    auto now = std::chrono::system_clock::from_time_t(getTimeNow());
    auto expires = date + std::chrono::seconds(maxAge);
    return checkReqStale(headers, expires, now, reqCon);
  }
  return true;
}
bool Cache::checkIfStale(const response &res, const reqControl &reqCon) {
  std::map<std::string, std::string> headers = res.get_header();
  std::map<std::string, std::string>::const_iterator cacheControlIterator =
      headers.find("Cache-Control");
  if (cacheControlIterator == headers.end()) {
    std::map<std::string, std::string>::const_iterator expiresIterator =
        headers.find("Expires");
    if (expiresIterator != headers.end()) {
      auto expires = stringToDate(expiresIterator->second);
      auto now = std::chrono::system_clock::from_time_t(getTimeNow());
      return checkReqStale(headers, expires, now, reqCon);
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
  return isMaxageStale(maxAgeString, headers, reqCon);
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
bool checkReqCachable(const request &req) {
  auto header = req.get_header();
  auto cacheIter = header.find("Cache-Control");
  if (cacheIter != header.end()) {
    std::string cacheControl = cacheIter->second;
    if (cacheControl.find("no-store") != std::string::npos) {
      return false;
    }
  }
  return true;
}
void Cache::updateCache(const request &req, response res) {
  if (!checkCachable(res) || !checkReqCachable(req)) {
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

reqControl checkControlInReq(const request &req) {
  auto header = req.get_header();
  auto cacheIter = header.find("Cache-Control");
  if (cacheIter == header.end()) {
    return reqControl();
  }
  std::string cacheControl = cacheIter->second;
  std::string maxAge = getMaxage(cacheControl);
  std::string maxStale = getInfoFromControl(cacheControl, "max-stale");
  std::string minFresh = getInfoFromControl(cacheControl, "min-fresh");
  return reqControl(maxAge, maxStale, minFresh);
}

cacheResponse Cache::getResponseFromCache(const request &req) {
  bool inCache = checkInCache(req);

  auto header = req.get_header();
  auto cacheIter = header.find("Cache-Control");
  if (cacheIter != header.end()) {
    std::string cacheControl = cacheIter->second;
    if (cacheControl.find("no-cache") != std::string::npos) {
      if (inCache) {
        auto resIter = cache.find(getHashKey(req));
        response res = resIter->second.first;
        return revalidate(res);
      }
    }
  }
  if (inCache) {
    auto resIter = cache.find(getHashKey(req));
    reqControl reqCon = checkControlInReq(req);
    response res = resIter->second.first;
    if (checkIfStale(res, reqCon)) {
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
