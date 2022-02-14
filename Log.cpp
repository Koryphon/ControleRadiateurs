#include "Log.h"
#include <cstdlib>
#include <map>
#include <set>

using namespace std;

static const set<string> kSupportedLanguages = {"fr_FR", "en_US"};

static const map<string, string> en_US_Messages = {
    {"noConfigFile",
     "A \033[4mconfig file\033[0m shall be given as argument - exiting"},

    {"configFileNotAccessible",
     "\033[4mConfig file\033[0m cannot be opened - exiting"},

    {"configFileLoaded", "Config file \033[93;4m%s\033[0m loaded"},

    {"location", "Location: \033[93;4m%s\033[0m"},

    {"unknownLocation", "Location: unknown"},

    {"connecting", "Connecting to host \033[93;4m%s:%s\033[0m"},

    {"connected", "Connected"},

    {"updateInterval", "Update interval of %ss"},

    {"profilesFound", "%s profile(s) found:"},

    {"profile", "  \033[32m%s\033[0m"},

    {"unresolvedAlias", "Unresolved alias - exiting"},

    {"aliasNotFound", "Alias profile \033[93;4m%s\033[0m not found"},

    {"aliasCircularity",
     "Circularity in alias definition: \033[93;4m%s\033[0m - exiting"},

    {"durationBadlyFormatted",
     "Profile \033[31m%s\033[0m, time \033[31m%s\033[0m, incorrect format for "
     "duration"},

    {"timeBadlyFormatted",
     "Profile \033[31m%s\033[0m, time \033[93;4m%s\033[0m, incorrect format"},

    {"badProfile", "Profile \033[31m%s\033[0m, object or string expected"},

    {"floatTemperatureOrObject", "Profile \033[31m%s\033[0m, time "
                                 "\033[31m%s\033[0m, float or object expected"},

    {"heaterDateNonExistingProfile",
     "Heater \033[31m%s\033[0m, date \033[31m%s\033[0m, profile "
     "\033[93;4m%s\033[0m does "
     "not exist"},

    {"heaterDateNoProfile", "Heater \033[31m%s\033[0m, date \033[31m%s\033[0m, "
                            "profile should be a string"},

    {"heaterDateNotObj",
     "Heater \033[31m%s\033[0m, \033[93;4mdate\033[0m should be an object"},

    {"heaterProfileNotString", "Heater \033[31m%s\033[0m, the "
                               "profile should be a string"},

    {"unknownProfile",
     "Heater \033[31m%s\033[0m, profile \033[93;4m%s\033[0m does not exist"},

    {"profileNotString", "Heater \033[31m%s\033[0m, profile "
                         "should be a string"},

    {"heaterNoProfile", "Heater \033[31m%s\033[0m, no defined profile"},

    {"heaterOffOrObject",
     "Heater \033[31m%s\033[0m, string \"off\" or object expected"},

    {"noHeater", "No heater defined - exiting"},

    {"heatersFound", "%s heaters defined:"},

    {"heater", "  \033[32m%s\033[0m: %s"},

    {"offStringExpected", "Heater \033[31m%s\033[0m, string \"off\" expected"},
};

static const map<string, string> fr_FR_Messages = {
    {"noConfigFile", "Un \033[4mfichier de configuration\033[0m devrait être "
                     "donné en argument - terminé"},

    {"configFileNotAccessible", "Le \033[4mfichier de configuration\033[0m ne "
                                "peut pas être ouvert - terminé"},

    {"configFileLoaded", "Fichier de configuration \033[93;4m%s\033[0m chargé"},

    {"location", "Lieu : \033[93;4m%s\033[0m"},

    {"unknownLocation", "Lieu: inconnu"},

    {"connecting", "Connexion à l'hôte \033[93;4m%s:%s\033[0m"},

    {"connected", "Connecté"},

    {"updateInterval", "Intervalle de mise à jour de %ss"},

    {"profilesFound", "%s profil(s) trouvé(s):"},

    {"profile", "  \033[32m%s\033[0m"},

    {"unresolvedAlias", "Alias non résolu - terminé"},

    {"aliasNotFound", "Alias de profil, \033[93;4m%s\033[0m, non trouvé"},

    {"aliasCircularity", "Circularité dans la définition des alias : "
                         "\033[93;4m%s\033[0m - terminé"},

    {"durationBadlyFormatted",
     "Profil \033[31m%s\033[0m, instant \033[31m%s\033[0m, format incorrect "
     "pour la durée"},

    {"timeBadlyFormatted",
     "Profil \033[31m%s\033[0m, instant \033[93;4m%s\033[0m, format incorrect"},

    {"badProfile", "Profil \033[31m%s\033[0m, objet ou chaine attendus"},

    {"floatTemperatureOrObject",
     "Profil \033[31m%s\033[0m, instant "
     "\033[31m%s\033[0m, flottant ou objet attendu"},

    {"heaterDateNonExistingProfile",
     "Radiateur \033[31m%s\033[0m, date \033[31m%s\033[0m, le profil "
     "\033[93;4m%s\033[0m n'existe pas"},

    {"heaterDateNoProfile",
     "Radiateur \033[31m%s\033[0m, date \033[31m%s\033[0m, "
     "le profil devrait être une chaine"},

    {"heaterDateNotObj", "Radiateur \033[31m%s\033[0m, la date "
                         "devrait être un objet"},

    {"heaterProfileNotString", "Le profil du Radiateur "
                               "\033[31m%s\033[0m devrait être une chaine"},

    {"unknownProfile", "Radiateur \033[31m%s\033[0m, le profil "
                       "\033[93;4m%s\033[0m n'existe pas"},

    {"profileNotString", "Radiateur \033[31m%s\033[0m, Le profil "
                         "devrait être une chaine"},

    {"heaterNoProfile", "Radiateur \033[31m%s\033[0m, pas de profil défini"},

    {"heaterOffOrObject",
     "Radiateur \033[31m%s\033[0m, chaine \"off\" ou objet attendu"},

    {"noHeater", "Aucun radiateur n'est défini - terminé"},

    {"heatersFound", "%s radiateurs définis :"},

    {"heater", "  \033[32m%s\033[0m : %s"},

    {"offStringExpected",
     "Radiateur \033[31m%s\033[0m, chaine \"off\" attendue"},
};

static const map<string, const map<string, string> &> localMapForLang = {
    {"fr_FR", fr_FR_Messages}, {"en_US", en_US_Messages}};

static const string &language() {
  static string lang = "";
  if (lang == "") {
    lang = getenv("LANG");
    if (int pos = lang.find('.')) {
      lang.erase(pos, lang.length() - pos);
    }

    if (!kSupportedLanguages.contains(lang)) {
      lang = "en_US";
    }
  }
  return lang;
}

static bool stringForKey(const string inKey, string &outMessage) {
  auto itLMFL = localMapForLang.find(language());
  if (itLMFL != localMapForLang.end()) {
    auto itMess = itLMFL->second.find(inKey);
    if (itMess != itLMFL->second.end()) {
      outMessage = itMess->second;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

static char buffer[1024];

void Log(Logger &inLogger, const char *inErrorId) {
  string message;
  if (stringForKey(inErrorId, message)) {
    inLogger << message << Logger::eol;
  } else {
    inLogger << "*** Internal error, message not found" << Logger::eol;
  }
}

void Log(Logger &inLogger, const char *inErrorId, const string &inObj) {
  string message;
  if (stringForKey(inErrorId, message)) {
    snprintf(buffer, 1024, message.c_str(), inObj.c_str());
    inLogger << buffer << Logger::eol;
  } else {
    inLogger << "*** Internal error, message not found" << Logger::eol;
  }
}

void Log(Logger &inLogger, const char *inErrorId, const string &inObj,
         const string &inAttributeName) {
  string message;
  if (stringForKey(inErrorId, message)) {
    snprintf(buffer, 1024, message.c_str(), inObj.c_str(),
             inAttributeName.c_str());
    inLogger << buffer << Logger::eol;
  } else {
    inLogger << "*** Internal error, message not found" << Logger::eol;
  }
}

void Log(Logger &inLogger, const char *inErrorId, const string &inObj,
         const string &inAttributeName, const string &inAttributeValue) {
  string message;
  if (stringForKey(inErrorId, message)) {
    snprintf(buffer, 1024, message.c_str(), inObj.c_str(),
             inAttributeName.c_str(), inAttributeValue.c_str());
    inLogger << buffer << Logger::eol;
  } else {
    inLogger << "*** Internal error, message not found" << Logger::eol;
  }
}
