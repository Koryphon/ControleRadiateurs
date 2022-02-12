#include "TimeStamp.h"

#include <sstream>

uint32_t minutesSinceMidnight() {
  time_t t = time(nullptr);
  tm tm = *localtime(&t);
  return tm.tm_hour * 60 + tm.tm_min;
}

float decimalMinutesSinceMidnight() {
  time_t t = time(nullptr);
  tm tm = *localtime(&t);
  return (float)tm.tm_hour * 60.0 + (float)tm.tm_min + (float)tm.tm_sec / 60.0;
}
