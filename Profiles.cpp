#include "Profiles.h"
#include "Log.h"
#include "TimeStamp.h"
#include <iostream>
#include <sstream>
#include <string>

template <typename Iter> Iter next(Iter iter) { return ++iter; }

bool timeStringToSeconds(const string &inTime, uint32_t &result) {
  for (int i = 0; i < inTime.length(); i++) {
    if (inTime[i] != ':' && (inTime[i] < '0' || inTime[i] > '9')) {
      return false;
    }
  }
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

bool ProfileAlias::check(Logger &inLogger, const string &inName,
                         set<string> &ioKnownProfiles) {
  if (ioKnownProfiles.contains(mAlias)) {
    Log(inLogger, "aliasCircularity", mAlias);
    exit(3);
    return false;
  } else {
    auto item = sProfiles.find(mAlias);
    if (item == sProfiles.end()) {
      Log(inLogger, "aliasNotFound", mAlias);
      return false;
    } else {
      ioKnownProfiles.insert(mAlias);
      const bool result =
          item->second->check(inLogger, inName, ioKnownProfiles);
      ioKnownProfiles.erase(mAlias);
      return result;
    }
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
                Log(inLogger, "durationBadlyFormatted", sKey, sTime);
              }
            } else if (value.is_number_float()) {
              float sTemp = value;
              tempList->add(t, sTemp);
            } else {
              Log(inLogger, "floatTemperatureOrObject", sKey, sTime);
            }
          } else {
            Log(inLogger, "timeBadlyFormatted", sKey, sTime);
          }
        }
        sProfiles[sKey] = tempList;
      } else {
        Log(inLogger, "badProfile", sKey);
      }
    }
  }
  /* Check aliases are ok */
  bool ok = true;
  set<string> knownProfiles;
  for (auto it : sProfiles) {
    ok &= it.second->check(inLogger, it.first, knownProfiles);
  }
  if (!ok) {
    Log(inLogger, "unresolvedAlias");
    exit(1);
  } else {
    Log(inLogger, "profilesFound", to_string(sProfiles.size()));
    for (auto it : sProfiles) {
      Log(inLogger, "profile", it.first);
    }
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
