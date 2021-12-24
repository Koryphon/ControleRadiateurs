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

#define CLIENT_ID "testMPP"
#define BROKER_ADDRESS "Arrakis.local"
#define MQTT_PORT 1883;
#define MQTT_TOPIC "EXAMPLE_TOPIC"

float setPoint = 18.0;
float pointInc = 0.1;
mqtt_client *iot_client;

Logger mainLogger;

void controlHeaters() {
  Heater::controlPool(iot_client, mainLogger);
  //   int message_id = 0;
  //   while (1) {
  //     sleep(10);
  //     std::string payload = to_string_p(setPoint, 2);
  //     iot_client->publish(&message_id, "heater0/setpoint", payload.size(),
  //                         payload.c_str());
  //     setPoint += pointInc;
  //     if (setPoint > 21)
  //       pointInc = -pointInc;
  //     if (setPoint < 18)
  //       pointInc = -pointInc;
  //   }
}

int main(int argc, char *argv[]) {
  int rc;

  char client_id[] = CLIENT_ID;
  char host[] = BROKER_ADDRESS;
  int port = MQTT_PORT;

  Logger::setOutStream(std::cout);

  std::ifstream file("config.json");
  nlohmann::json config;
  file >> config;

  mainLogger << "Fichier de configuration chargé" << Logger::eol;
  mainLogger << "Lieu : ";
  auto found = config.find("location");
  if (found != config.end()) {
    std::string location = found.value();
    mainLogger << location << Logger::eol;
  } else {
    mainLogger << "<inconnu>" << Logger::eol;
  }

  Profile::parse(config, mainLogger);
  Heater::parse(config, mainLogger);

  mosqpp::lib_init();

  if (argc > 1)
    strcpy(host, argv[1]);

  iot_client = new mqtt_client(client_id, host, port);

  int message_id = 0;
  iot_client->subscribe(&message_id, "heater0/temperature");
  iot_client->subscribe(&message_id, "heater0/setpoint");
  iot_client->subscribe(&message_id, "heater1/setpoint");

  std::thread updateSetpoint(controlHeaters);

  while (1) {
    rc = iot_client->loop();
    if (rc) {
      iot_client->reconnect();
      iot_client->subscribe(&message_id, "heater0/temperature");
      iot_client->subscribe(&message_id, "heater0/setpoint");
      iot_client->subscribe(&message_id, "heater1/setpoint");
    }
  }

  updateSetpoint.join();

  mosqpp::lib_cleanup();

  return 0;
}
