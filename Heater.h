#ifndef __HEATER_H__
#define __HEATER_H__

#include "Logger.h"
#include "mqtt.h"
#include <ctime>
#include <nlohmann/json.hpp>
#include <set>
#include <string>

using namespace std;

class Heater {
public:
  typedef enum { AUTOMATIC, MANUAL, STOP } HeaterMode;

  /* set of created heaters */
  static set<Heater *> sHeaters;
  string mName;
  string mProfile;
  float mOffset;
  map<time_t, string> mProfileForDate;

public:
  Heater(const string &inName, const string &inProfile,
         const map<time_t, string> &inProfilesForDate, const float inOffset)
      : mName(inName), mProfile(inProfile), mOffset(inOffset),
        mProfileForDate(inProfilesForDate) {}
  const string &name() { return mName; }
  static void parse(nlohmann::json &inConfig, Logger &inLogger);
  static void controlPool(mqtt_client *const inClient, Logger &inLogger);
  void control(mqtt_client *const inClient);
  void setMode(mqtt_client *const inClient, const HeaterMode inMode);
  void setOffset(mqtt_client *const inClient);
};

#endif