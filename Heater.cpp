#include "Heater.h"
#include "Cfg.h"
#include "Logger.h"
#include "Profiles.h"
#include "TimeStamp.h"
#include "Util.h"
#include <iostream>
#include <unistd.h>

Logger mHeaterLogger;

set<Heater *> Heater::sHeaters;

void Heater::parse(nlohmann::json &inConfig, Logger &inLogger) {
  auto found = inConfig.find("heaters");
  if (found != inConfig.end()) {
    for (auto &[key, value] : found.value().items()) {
      string sKey = key;
      //      cout << sKey << endl;
      if (value.is_string()) {
        string sVal = value;
        if (sVal == "off") {
        } else {
          inLogger << "La clé " << sVal
                   << "n'est pas attendue pour le radiateur " << sKey
                   << Logger::eol;
        }
      } else if (value.is_object()) {
        auto profile = value.find("profile");
        if (profile != value.end()) {
          if (profile->is_string()) {
            string sProfile = *profile;
            float dummy;
            if (Profile::temperatureForProfile(sProfile, dummy)) {
              sHeaters.insert(new Heater(sKey, sProfile));
            } else {
              inLogger << "Le profil " << sProfile
                       << " utilisé par le radiateur " << sKey
                       << " n'existe pas" << Logger::eol;
            }
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
  }
  for (auto h : sHeaters) {
    inLogger << h->mName << " : " << h->mProfile << Logger::eol;
  }
}

void Heater::controlPool(mqtt_client *const inClient, Logger &inLogger) {
  /* Compute the interval between 2 heater control call */
  uint32_t interval = kCycle / sHeaters.size();
  if (interval < kMinInterval)
    interval = kMinInterval;
  if (interval > kMaxInterval)
    interval = kMaxInterval;
  inLogger << "Intervalle de mise à jour à " << interval << "s" << Logger::eol;
  while (1) {
    for (auto h : sHeaters) {
      h->setMode(inClient, AUTOMATIC);
      h->control(inClient);
      sleep(interval);
    }
  }
}

void Heater::control(mqtt_client *const inClient) {
  int messageId = 0;
  float temp;
  if (Profile::temperatureForProfile(mProfile, temp)) {
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
  if (inMode != mMode) {
    string topic = mName + "/mode";
    mMode = inMode;
    inClient->publish(&messageId, topic.c_str(), payload.size(),
                      payload.c_str());
  }
}