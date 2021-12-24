#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <iostream>
#include <mutex>
#include <sstream>

using namespace std;

class Logger {
  static ostream *mOutstream;
  static mutex mLock;
  stringstream mBuffer;
  void timestamp();

public:
  static const char eol = 0x0A;
  Logger() {}
  static void setOutStream(ostream &inOutstream) { mOutstream = &inOutstream; }
  Logger &operator<<(const string &inData);
  Logger &operator<<(const float inData);
  Logger &operator<<(const uint32_t inData);
  Logger &operator<<(const int inData);
  Logger &operator<<(const char inData);
};

#endif