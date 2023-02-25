#include "common.h"
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
  auto now = std::chrono::system_clock::now();
  return std::chrono::system_clock::to_time_t(now);
}
