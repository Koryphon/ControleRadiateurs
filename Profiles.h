/*
 * Classe permettant de stocker un profil de chauffage
 */

#ifndef __PROFILE_H__
#define __PROFILE_H__

#include "Logger.h"
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>

using namespace std;

class BaseTimedTemp {
  uint32_t mMinutes;

public:
  BaseTimedTemp(const uint32_t inMinutes) : mMinutes(inMinutes) {}
  virtual float temperature() const { return 0.0; }
  uint32_t minutes() const { return mMinutes; }
  virtual void print(ostream &o);
  friend bool operator<(const BaseTimedTemp &inOp1, const BaseTimedTemp &inOp2);
};

class TimedTemp : public BaseTimedTemp {
  float mTemperature;

public:
  TimedTemp(const uint32_t inMinutes, const float inTemperature)
      : BaseTimedTemp(inMinutes), mTemperature(inTemperature) {}
  virtual float temperature() const { return mTemperature; }
  virtual void print(ostream &o);
};

class SlopedTimedTemp : public BaseTimedTemp {
  float mDuration;
  float mStartTemperature;
  float mEndTemperature;

public:
  SlopedTimedTemp(const uint32_t inMinutes, const uint32_t inDuration,
                  const float inStartTemperature, const float inEndTemperature)
      : BaseTimedTemp(inMinutes), mDuration((float)inDuration),
        mStartTemperature(inStartTemperature),
        mEndTemperature(inEndTemperature) {}
  virtual float temperature() const;
  virtual void print(ostream &o);
};

class Profile {
protected:
  static map<string, Profile *> sProfiles;

public:
  static void parse(nlohmann::json &inConfig, Logger &inLogger);
  static bool temperatureForProfile(const string &inProfile, float &outTemp);
  Profile() {}
  virtual bool check(Logger &inLogger, const string &inName,
                     set<string> &ioKnownProfiles) = 0;
  virtual bool temperature(float &outTemp) = 0;
};

class ProfileAlias : public Profile {
  string mAlias;

public:
  ProfileAlias(string &inAlias) : Profile() { mAlias = inAlias; }
  virtual bool check(Logger &inLogger, const string &inName,
                     set<string> &ioKnownProfiles);
  virtual bool temperature(float &outTemp);
};

class ProfileTemp : public Profile {
  set<BaseTimedTemp *> mTemps;

public:
  ProfileTemp() : Profile() {}
  void add(const uint32_t inMinutes, const float inTemperature);
  void add(const uint32_t inMinutes, const uint32_t inDuration,
           const float inStartTemperature, const float inEndTemperature);
  virtual bool check(Logger &inLogger, const string &inName,
                     set<string> &ioKnownProfiles) {
    return true;
  }
  virtual bool temperature(float &outTemp);
  friend ostream &operator<<(ostream &s, ProfileTemp &pt);
};

#endif