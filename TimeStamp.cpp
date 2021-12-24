#include "TimeStamp.h"

#include <sstream>

uint32_t minutesSinceMidnight() {
  time_t t = time(nullptr);
  tm tm = *localtime(&t);
  return tm.tm_hour * 60 + tm.tm_min;
}