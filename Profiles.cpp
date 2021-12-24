#include "Profiles.h"
#include "TimeStamp.h"
#include <iostream>
#include <sstream>
#include <string>

bool operator<(const TimedTemp &inOp1, const TimedTemp &inOp2) {
  return inOp1.mMinutes < inOp2.mMinutes;
}

ostream &operator<<(ostream &s, TimedTemp &tt) {
  s << tt.mMinutes << " : " << tt.mTemperature;
  return s;
}

ostream &operator<<(ostream &s, ProfileTemp &pt) {
  for (auto it : pt.mTemps) {
    TimedTemp tt = it;
    s << "    " << tt << endl;
  }
  return s;
}

map<string, Profile *> Profile::sProfiles;

bool ProfileAlias::check(Logger &inLogger) {
  auto item = sProfiles.find(mAlias);
  if (item == sProfiles.end()) {
    inLogger << "Le profil \"" << mAlias << "\" n'existe pas" << Logger::eol;
    return false;
  } else {
    return item->second->check(inLogger);
  }
}

void ProfileTemp::add(const uint32_t inMinutes, const float inTemperature) {
  TimedTemp slot(inMinutes, inTemperature);
  mTemps.insert(slot);
}

void Profile::parse(nlohmann::json &inConfig, Logger &inLogger) {
  auto found = inConfig.find("profiles");
  if (found != inConfig.end()) {
    for (auto &[key, value] : found.value().items()) {
      string sKey = key;
      //      cout << sKey << endl;
      if (value.is_string()) {
        string sVal = value;
        ProfileAlias *alias = new ProfileAlias(sVal);
        sProfiles[sKey] = alias;
        //        cout << "    " << sVal << endl;
      } else if (value.is_object()) {
        ProfileTemp *tempList = new ProfileTemp();
        for (auto &[key, value] : value.items()) {
          string sTime = key;
          float sTemp = value;
          //          cout << sTime << ", " << sTemp << endl;
          if (int pos = sTime.find(':')) {
            stringstream sHours;
            sHours << sTime.substr(0, pos);
            stringstream sMinutes;
            sMinutes << sTime.substr(pos + 1);
            uint32_t hours;
            sHours >> hours;
            uint32_t minutes;
            sMinutes >> minutes;
            //            cout << hours * 60 + minutes << endl;
            tempList->add(hours * 60 + minutes, sTemp);
          }
        }
        sProfiles[sKey] = tempList;
        //       cout << *tempList;
      } else {
        inLogger << "Le profil " << sKey << " doit être un alias ou une liste"
                 << Logger::eol;
      }
    }
  }
  /* Check aliases are ok */
  bool ok = true;
  for (auto it : sProfiles) {
    ok &= it.second->check(inLogger);
  }
  if (!ok) {
    inLogger << "Terminé" << Logger::eol;
    exit(1);
  }
}

bool Profile::temperatureForProfile(const string &inProfile, float &outTemp) {
  auto item = sProfiles.find(inProfile);
  if (item != sProfiles.end()) {
    return item->second->temperature(outTemp);
  } else {
    return false;
  }
}

bool ProfileTemp::temperature(float &outTemp) {
  const uint32_t minutes = minutesSinceMidnight();
  for (auto temp = mTemps.rbegin(); temp != mTemps.rend(); temp++) {
    if (temp->minutes() <= minutes) {
      outTemp = temp->temperature();
      return true;
    }
  }
  return false;
}

bool ProfileAlias::temperature(float &outTemp) {
  auto item = sProfiles.find(mAlias);
  if (item != sProfiles.end()) {
    return item->second->temperature(outTemp);
  } else {
    return false;
  }
}
