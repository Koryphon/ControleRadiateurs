#include "mqtt.h"
#include "Logger.h"
#ifdef DEBUG
#include <iostream>
#endif

Logger mMQTTLogger;

mqtt_client::mqtt_client(const char *id, const char *host, int port)
    : mosquittopp(id) {
  int keepalive = DEFAULT_KEEP_ALIVE;
  connect(host, port, keepalive);
}

mqtt_client::~mqtt_client() {}

void mqtt_client::on_connect(int rc) {
  if (!rc) {
#ifdef DEBUG
    mMQTTLogger << "Connected - code " << rc << Logger::eol;
#endif
  }
}

void mqtt_client::on_subscribe(int mid, int qos_count, const int *granted_qos) {
#ifdef DEBUG
  mMQTTLogger << "Subscription succeeded." << Logger::eol;
#endif
}

void mqtt_client::on_message(const struct mosquitto_message *message) {
  int payload_size = MAX_PAYLOAD + 1;
  char buf[payload_size];

#ifdef DEBUG2
  mMQTTLogger << "MESSAGE - " << message->topic << ": "
              << (char *)(message->payload) << Logger::eol;
#endif
}
