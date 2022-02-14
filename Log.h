#ifndef __LOG_H__
#define __LOG_H__

#include "Logger.h"
#include <string>

void Log(Logger &inLogger, const char *inErrorId);

void Log(Logger &inLogger, const char *inErrorId, const string &inObj);

void Log(Logger &inLogger, const char *inErrorId, const string &inObj,
         const string &inAttributeName);

void Log(Logger &inLogger, const char *inErrorId, const string &inObj,
         const string &inAttributeName, const string &inAttributeValue);

#endif