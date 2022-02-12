#include "Heater.h"
#include "Logger.h"
#include "Profiles.h"
#include "TimeStamp.h"
#include "Util.h"
#include "mqtt.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <unistd.h>

#define CLIENT_ID "ctrlheaters"
#define BROKER_ADDRESS "localhost"
//#define BROKER_ADDRESS "Arrakis.local"
// #define BROKER_ADDRESS "raspberrypi.local"
#define MQTT_PORT 1883;
#define MQTT_TOPIC "EXAMPLE_TOPIC"

mqtt_client *iot_client;

Logger mainLogger;

void controlHeaters() { Heater::controlPool(iot_client, mainLogger); }

int main(int argc, char *argv[]) {
  int rc;

  string client_id = CLIENT_ID;
  string host = BROKER_ADDRESS;
  int port = MQTT_PORT;

  Logger::setOutStream(std::cout);

  if (argc != 2) {
    mainLogger << "A config file shall be given as argument - exiting"
               << Logger::eol;
    return 1;
  }

  string configFileName = argv[1];

  std::ifstream file(configFileName);
  if (!file.is_open()) {
    mainLogger << "Config file cannot be opened - exiting" << Logger::eol;
    return 2;
  }

  nlohmann::json config;
  file >> config;

  mainLogger << "Configuration file loaded" << Logger::eol;
  mainLogger << "Location : ";
  auto found = config.find("location");
  if (found != config.end()) {
    string location = found.value();
    mainLogger << location << Logger::eol;
  } else {
    mainLogger << "<unkown>" << Logger::eol;
  }

  /* Get the mqtt broker host and port if any */
  found = config.find("host");
  if (found != config.end()) {
    host = found.value();
  }

  found = config.find("port");
  if (found != config.end()) {
    port = found.value();
  }

  Profile::parse(config, mainLogger);
  Heater::parse(config, mainLogger);

  mosqpp::lib_init();

  mainLogger << "Connecting to host: " << host << ':' << port << Logger::eol;

  iot_client = new mqtt_client(client_id.c_str(), host.c_str(), port);

  int message_id = 0;
  std::thread updateSetpoint(controlHeaters);

  while (1) {
    rc = iot_client->loop();
    if (rc) {
      iot_client->reconnect();
    }
  }

  updateSetpoint.join();

  mosqpp::lib_cleanup();

  return 0;
}
