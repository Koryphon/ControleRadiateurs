#include "Profiles.h"
#include "TimeStamp.h"
#include <iostream>
#include <sstream>
#include <string>

template <typename Iter> Iter next(Iter iter) { return ++iter; }

bool timeStringToSeconds(const string &inTime, uint32_t &result) {
  if (int pos = inTime.find(':')) {
    stringstream hours;
    hours << inTime.substr(0, pos);
    stringstream minutes;
    minutes << inTime.substr(pos + 1);
    uint32_t h;
    hours >> h;
    uint32_t m;
    minutes >> m;
    if (h >= 0 && h <= 23 && m >= 0 && m <= 59) {
      result = h * 60 + m;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool operator<(const BaseTimedTemp &inOp1, const BaseTimedTemp &inOp2) {
  return inOp1.mMinutes < inOp2.mMinutes;
}

ostream &operator<<(ostream &s, BaseTimedTemp *tt) {
  tt->print(s);
  return s;
}

ostream &operator<<(ostream &s, TimedTemp *tt) {
  tt->print(s);
  return s;
}

ostream &operator<<(ostream &s, SlopedTimedTemp *tt) {
  tt->print(s);
  return s;
}

ostream &operator<<(ostream &s, ProfileTemp &pt) {
  for (auto it : pt.mTemps) {
    BaseTimedTemp *tt = it;
    s << "    ";
    tt->print(s);
    s << endl;
  }
  return s;
}

void BaseTimedTemp::print(ostream &o) { o << "-> " << mMinutes; }

void TimedTemp::print(ostream &o) {
  BaseTimedTemp::print(o);
  o << " : " << mTemperature;
}

void SlopedTimedTemp::print(ostream &o) {
  BaseTimedTemp::print(o);
  o << " : " << mStartTemperature << " --(" << mDuration << ")-- "
    << mEndTemperature;
}

float SlopedTimedTemp::temperature() const {
  const float mins = decimalMinutesSinceMidnight() - (float)minutes();
  const float deltaTemp = mEndTemperature - mStartTemperature;
  if (mDuration == 0.0 || mins >= mDuration) {
    return mEndTemperature;
  } else {
    return mStartTemperature + deltaTemp * mins / mDuration;
  }
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
  mTemps.insert(new TimedTemp(inMinutes, inTemperature));
}

void ProfileTemp::add(const uint32_t inMinutes, const uint32_t inDuration,
                      const float inStartTemperature,
                      const float inEndTemperature) {
  mTemps.insert(new SlopedTimedTemp(inMinutes, inDuration, inStartTemperature,
                                    inEndTemperature));
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
          uint32_t t;
          if (timeStringToSeconds(sTime, t)) {
            if (value.is_object()) {
              // inLogger << "Slope trouvée" << Logger::eol;
              auto first = value.begin();
              uint32_t duration;
              if (timeStringToSeconds(first.key(), duration)) {
                // inLogger << duration << Logger::eol;
                float startTemp = first.value().at(0);
                float endTemp = first.value().at(1);
                // inLogger << startTemp << ", " << endTemp << Logger::eol;
                tempList->add(t, duration, startTemp, endTemp);
              } else {
                inLogger << "The duration of the time \"" << sTime
                         << "\" of the profile \"" << sKey
                         << "\" is incorrectly formatted" << Logger::eol;
              }
            } else if (value.is_number_float()) {
              float sTemp = value;
              tempList->add(t, sTemp);
            } else {
              inLogger << "At time \"" << sTime
                       << "\", the value should be a temperature in float or "
                          "an object."
                       << Logger::eol;
            }
          } else {
            inLogger << "The time \"" << sTime << "\" of profile \"" << sKey
                     << "\" is incorrectly formatted" << Logger::eol;
          }
        }
        sProfiles[sKey] = tempList;
        // cout << *tempList;
      } else {
        inLogger << "The profile \"" << sKey
                 << "\" must be an alias or an object" << Logger::eol;
      }
    }
  }
  /* Check aliases are ok */
  bool ok = true;
  for (auto it : sProfiles) {
    ok &= it.second->check(inLogger);
  }
  if (!ok) {
    inLogger << "Unresolved alias(es) in profiles. Exiting" << Logger::eol;
    exit(1);
  } else {
    inLogger << (uint32_t)sProfiles.size() << " profiles found: ";
    int count = 0;
    for (auto it : sProfiles) {
      inLogger << '"' << it.first << '"';
      count++;
      if (count < sProfiles.size()) {
        inLogger << ", ";
      }
    }
    inLogger << Logger::eol;
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
    BaseTimedTemp *tempObj = *temp;
    if (tempObj->minutes() <= minutes) {
      outTemp = tempObj->temperature();
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
