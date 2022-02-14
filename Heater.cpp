#include "Heater.h"
#include "Cfg.h"
#include "Log.h"
#include "Logger.h"
#include "Profiles.h"
#include "TimeStamp.h"
#include "Util.h"
#include <iostream>
#include <unistd.h>

/*----------------------------------------------------------------------------
 * Static member of the Heater class. Used to store the heaters
 * instanciated after reading the configuration file.
 */
set<Heater *> Heater::sHeaters;

/*----------------------------------------------------------------------------
 * Returns a time_t for the date as string. Used to convert the dates of the
 * json object "date" to seconds since the first of January 1900.
 */
static time_t getTime(const string &inDate) {
  struct tm tmp = {0};
  strptime(inDate.c_str(), "%Y-%m-%d %H:%M", &tmp);
  return mktime(&tmp);
}

/*----------------------------------------------------------------------------
 * Retrieves information from the "heaters" section of the json and creates the
 * corresponding heaters. TODO: split into several functions.
 */
void Heater::parse(nlohmann::json &inConfig, Logger &inLogger) {
  /*
   * Look for "heaters" section
   */
  auto found = inConfig.find("heaters");
  if (found != inConfig.end()) {
    /*
     * enumerate heaters
     */
    for (auto &[key, value] : found.value().items()) {
      /*
       * key is the heater name
       */
      string sKey = key;
      //      cout << sKey << endl;
      if (value.is_string()) {
        /*
         * If the value is a string, it shall be "off"
         * In this case, the heater is not instantiated.
         */
        string sVal = value;
        if (sVal == "off") {
        } else {
          Log(inLogger, "offStringExpected", sKey);
          inLogger << "The key " << sVal << "is not expected for the heater "
                   << sKey << Logger::eol;
        }
      } else if (value.is_object()) {
        /*
         * The value is an object. So we look for "location" and "profile"
         */
        string sRoom = "";
        auto room = value.find("location");
        if (room != value.end()) {
          if (room->is_string()) {
            sRoom = *room;
          } else {
          }
        }

        auto profile = value.find("profile");
        if (profile != value.end()) {
          if (profile->is_string()) {
            string sProfile = *profile;
            float dummy;
            if (Profile::temperatureForProfile(sProfile, dummy)) {
              /* Look for the offset if any */
              float o = 0.0;
              auto offset = value.find("offset");
              if (offset != value.end()) {
                o = *offset;
              }

              /* Récupère la section date qui définit un profil en fct de la
               * date */
              map<time_t, string> datedProfiles;
              auto date = value.find("date");
              if (date != value.end()) {
                if (date->is_object()) {
                  for (auto &[key, value] : date.value().items()) {
                    /* key = date, value = profile de chauffe */
                    time_t time = getTime(key);
                    // inLogger << key << " : " << time << Logger::eol;
                    if (value.is_string()) {
                      string datedP = value;
                      if (Profile::temperatureForProfile(datedP, dummy)) {
                        datedProfiles.insert(
                            pair<time_t, string>(time, datedP));
                      } else {
                        Log(inLogger, "heaterDateNonExistingProfile", sKey, key,
                            datedP);
                      }
                    } else {
                      Log(inLogger, "heaterDateNoProfile", sKey, key);
                    }
                  }
                } else {
                  Log(inLogger, "heaterDateNotObj", sKey);
                }
              }

              sHeaters.insert(
                  new Heater(sKey, sRoom, sProfile, datedProfiles, o));
            } else {
              Log(inLogger, "unknownProfile", sKey, sProfile);
            }
          } else {
            Log(inLogger, "profileNotString", sKey);
          }
        } else {
          Log(inLogger, "heaterNoProfile", sKey);
        }
      } else {
        Log(inLogger, "heaterOffOrObject", sKey);
      }
    }
  } else {
    Log(inLogger, "noHeater");
    exit(4);
  }
  // for (auto h : sHeaters) {
  //   inLogger << h->mName << " : " << h->mProfile << Logger::eol;
  // }
  Log(inLogger, "heatersFound", to_string(sHeaters.size()));
  for (auto h : sHeaters) {
    Log(inLogger, "heater", h->mName, h->mRoom);
  }
}

void Heater::controlPool(mqtt_client *const inClient, Logger &inLogger) {
  /* Compute the interval between 2 heater control call */
  uint32_t interval = kCycle / sHeaters.size() / 2;
  if (interval < kMinInterval)
    interval = kMinInterval;
  if (interval > kMaxInterval)
    interval = kMaxInterval;
  Log(inLogger, "updateInterval", to_string(interval));
  while (1) {
    // for (auto h : sHeaters) {
    //   h->setOffset(inClient);
    //   sleep(interval);
    // }
    for (auto h : sHeaters) {
      h->control(inClient);
      sleep(interval);
      h->setMode(inClient, AUTOMATIC);
      sleep(interval);
    }
  }
}

void Heater::control(mqtt_client *const inClient) {
  int messageId = 0;
  float temp;
  /* Check if current time correspond to a timed profile */
  time_t currentDate = time(NULL);
  bool found = false;
  for (auto iter = mProfileForDate.rbegin(); iter != mProfileForDate.rend();
       ++iter) {
    //    cout << currentDate << " / " << iter->first << " : " << iter->second
    //         << endl;
    time_t activationDate = iter->first;
    if (currentDate >= activationDate) {
      string profile = iter->second;
      if (Profile::temperatureForProfile(profile, temp)) {
        //        cout << "Trouvé : " << activationDate;
        found = true;
        string topic = mName + "/setpoint";
        string payload = to_string_p(temp, 2);
        //        cout << " - " << topic << " : " << payload << endl;
        inClient->publish(&messageId, topic.c_str(), payload.size(),
                          payload.c_str());
        break;
      }
    }
  }

  if (!found && Profile::temperatureForProfile(mProfile, temp)) {
    // if (mName == "heater1") {
    //   cout << "À " << decimalMinutesSinceMidnight() << ", temp = " << temp
    // }
    string topic = mName + "/setpoint";
    string payload = to_string_p(temp, 2);
    inClient->publish(&messageId, topic.c_str(), payload.size(),
                      payload.c_str());
  }
}

void Heater::setMode(mqtt_client *const inClient, const HeaterMode inMode) {
  int messageId = 0;
  string payload;
  switch (inMode) {
  case AUTOMATIC:
    payload = "auto";
    break;
  case MANUAL:
    payload = "manual";
    break;
  case STOP:
    payload = "stop";
    break;
  default:
    return;
  }
  string topic = mName + "/mode";
  inClient->publish(&messageId, topic.c_str(), payload.size(), payload.c_str());
}

void Heater::setOffset(mqtt_client *const inClient) {
  int messageId = 0;
  string topic = mName + "/offset";
  string payload = to_string_p(mOffset, 2);
  inClient->publish(&messageId, topic.c_str(), payload.size(), payload.c_str());
}
