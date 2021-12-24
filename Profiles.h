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

class TimedTemp {
  uint32_t mMinutes;
  float mTemperature;

public:
  TimedTemp(const uint32_t inMinutes, const float inTemperature)
      : mMinutes(inMinutes), mTemperature(inTemperature) {}
  float temperature() const { return mTemperature; }
  uint32_t minutes() const { return mMinutes; }
  friend bool operator<(const TimedTemp &inOp1, const TimedTemp &inOp2);
  friend ostream &operator<<(ostream &s, TimedTemp &tt);
};

class Profile {
protected:
  static map<string, Profile *> sProfiles;

public:
  static void parse(nlohmann::json &inConfig, Logger &inLogger);
  static bool temperatureForProfile(const string &inProfile, float &outTemp);
  Profile() {}
  virtual bool check(Logger &inLogger) = 0;
  virtual bool temperature(float &outTemp) = 0;
};

class ProfileAlias : public Profile {
  string mAlias;

public:
  ProfileAlias(string &inAlias) : Profile() { mAlias = inAlias; }
  virtual bool check(Logger &inLogger);
  virtual bool temperature(float &outTemp);
};

class ProfileTemp : public Profile {
  set<TimedTemp> mTemps;

public:
  ProfileTemp() : Profile() {}
  void add(const uint32_t inMinutes, const float inTemperature);
  virtual bool check(Logger &inLogger) { return true; }
  virtual bool temperature(float &outTemp);
  friend ostream &operator<<(ostream &s, ProfileTemp &pt);
};

#endif