#include "time_util.h"
std::chrono::system_clock::time_point stringToDate(const std::string &date) {
  std::tm time{};
  std::istringstream dateStream(date);
  dateStream >> std::get_time(&time, "%a, %d %b %Y %H:%M:%S %Z");

  std::chrono::system_clock::time_point timePoint =
      std::chrono::system_clock::from_time_t(std::mktime(&time));
  return timePoint;
}
std::string dateToString(const std::chrono::system_clock::time_point &date) {
  std::stringstream ss;
  std::time_t t = std::chrono::system_clock::to_time_t(date);
  struct std::tm tm = *std::gmtime(&t); // Convert to UTC time
  ss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
  return ss.str();
}
std::time_t getTimeNow() {
  // Get the current time
  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

  // Convert to UTC time
  std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  std::tm utc_tm = *gmtime(&now_time);
  std::chrono::system_clock::time_point utc_now =
      std::chrono::system_clock::from_time_t(std::mktime(&utc_tm));

  return std::chrono::system_clock::to_time_t(utc_now);
}
