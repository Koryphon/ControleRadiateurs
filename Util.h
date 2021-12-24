#ifndef __UTIL_H__
#define __UTIL_H__

#include <sstream>

template <typename T>
std::string to_string_p(const T a_value, const int n = 6) {
  std::ostringstream out;
  out.precision(n);
  out << std::fixed << a_value;
  return out.str();
}

#endif