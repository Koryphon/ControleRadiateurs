#include "Heater.h"
#include "Cfg.h"
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
          inLogger << "The key " << sVal << "is not expected for the heater "
                   << sKey << Logger::eol;
        }
      } else if (value.is_object()) {
        /*
         * The value is an object. So we look for "profile"
         */
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
                        inLogger << "Radiateur " << sKey << ", date " << key
                                 << ", le profil " << datedP << " n'existe pas"
                                 << Logger::eol;
                      }
                    } else {
                      inLogger << "Radiateur " << sKey << ", date " << key
                               << " : Profil attendu" << Logger::eol;
                    }
                  }
                } else {
                  inLogger << "Radiateur " << sKey
                           << "L'attribut date devrait être un objet"
                           << Logger::eol;
                }
              }

              sHeaters.insert(new Heater(sKey, sProfile, datedProfiles, o));
            } else {
              inLogger << "Le profil " << sProfile
                       << " utilisé par le radiateur " << sKey
                       << " n'existe pas" << Logger::eol;
            }
          } else {
            inLogger << "The profile of the heater " << sKey
                     << " should be a string" << Logger::eol;
          }
        } else {
          inLogger << "Aucun profil défini pour le radiateur " << sKey
                   << Logger::eol;
        }
      } else {
        inLogger << "Le radiateur " << sKey
                 << " doit être \"off\" ou avoir un profil" << Logger::eol;
      }
    }
  } else {
    inLogger << "Aucun radiateur défini" << Logger::eol;
  }
  // for (auto h : sHeaters) {
  //   inLogger << h->mName << " : " << h->mProfile << Logger::eol;
  // }
  inLogger << (uint32_t)sHeaters.size() << " heaters found";
  if (sHeaters.size() > 0) {
    inLogger << ':';
  }
  inLogger << Logger::eol;
  for (auto h : sHeaters) {
    inLogger << "  " << h->mName << ": " << h->mProfile << Logger::eol;
  }
}

void Heater::controlPool(mqtt_client *const inClient, Logger &inLogger) {
  /* Compute the interval between 2 heater control call */
  uint32_t interval = kCycle / sHeaters.size() / 2;
  if (interval < kMinInterval)
    interval = kMinInterval;
  if (interval > kMaxInterval)
    interval = kMaxInterval;
  inLogger << "Intervalle de mise à jour à " << interval << "s" << Logger::eol;
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
