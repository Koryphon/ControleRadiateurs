#include "Logger.h"
#include <chrono>
#include <ctime>
#include <iomanip>

ostream *Logger::mOutstream = NULL;
mutex Logger::mLock;

void Logger::timestamp() {
  if (mBuffer.str().size() == 0) {
    stringstream ts;
    time_t t = time(nullptr);
    tm tm = *localtime(&t);
    ts << put_time(&tm, "%e %b %H:%M:%S : ");
    mBuffer << ts.str();
  }
}

Logger &Logger::operator<<(const string &inData) {
  timestamp();
  mBuffer << inData;
  return *this;
}

Logger &Logger::operator<<(const float inData) {
  timestamp();
  mBuffer << inData;
  return *this;
}

Logger &Logger::operator<<(const uint32_t inData) {
  timestamp();
  mBuffer << inData;
  return *this;
}

Logger &Logger::operator<<(const int inData) {
  timestamp();
  mBuffer << inData;
  return *this;
}

Logger &Logger::operator<<(const char inData) {
  timestamp();
  if (inData == eol) {
    mBuffer << endl;
    mLock.lock();
    *mOutstream << mBuffer.str();
    mBuffer.str(string());
    mLock.unlock();
  } else {
    mBuffer << inData;
  }
  return *this;
}
