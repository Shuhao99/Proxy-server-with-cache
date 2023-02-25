#ifndef TIME_UTIL_H
#define TIME_UTIL_H
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

std::chrono::system_clock::time_point stringToDate(const std::string &date);
std::string dateToString(const std::chrono::system_clock::time_point &date);
std::time_t getTimeNow();
#endif
