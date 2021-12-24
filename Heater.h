#ifndef __HEATER_H__
#define __HEATER_H__

#include "Logger.h"
#include "mqtt.h"
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
  HeaterMode mMode;

public:
  Heater(const string &inName, const string &inProfile)
      : mName(inName), mProfile(inProfile), mMode(MANUAL) {}
  const string &name() { return mName; }
  static void parse(nlohmann::json &inConfig, Logger &inLogger);
  static void controlPool(mqtt_client *const inClient, Logger &inLogger);
  void control(mqtt_client *const inClient);
  void setMode(mqtt_client *const inClient, const HeaterMode inMode);
};

#endif