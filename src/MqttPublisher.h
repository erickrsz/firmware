#pragma once
#include "DhtSensor.h"

class MqttPublisher {
public:
  static void begin();
  static void reconnectIfNeeded();
  static void publishReading(const SensorReading& reading);
};
