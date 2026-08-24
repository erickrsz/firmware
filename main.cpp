#include <Arduino.h>
#include "config.h"
#include "WifiManager.h"
#include "DhtSensor.h"
#include "MqttPublisher.h"

unsigned long lastReadTime = 0;

void setup() {
  Serial.begin(115200);

  DhtSensor::begin();
  WifiManager::connect();
  MqttPublisher::begin();
}

void loop() {
  if (!WifiManager::isConnected()) {
    WifiManager::connect();
  }

  MqttPublisher::reconnectIfNeeded();

  if (lastReadTime == 0 || millis() - lastReadTime >= READ_INTERVAL_MS) {
    lastReadTime = millis();

    SensorReading reading = DhtSensor::read();
    MqttPublisher::publishReading(reading);
  }
}
