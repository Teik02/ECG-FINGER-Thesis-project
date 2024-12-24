/**
 * @brief Controls the power shutdown mode for the AD8232 sensor.
 * 
 * This function sets the shutdown pin (SDN) of the AD8232 sensor to either HIGH or LOW
 * based on the provided power mode. When the power mode is true, the sensor is powered down.
 * When the power mode is false, the sensor is powered up.
 *
 * @param powerMode A boolean value indicating the desired power mode.
 *                  - true: Power down the sensor (set SDN to HIGH).
 *                  - false: Power up the sensor (set SDN to LOW).
 * @return void
 */
extern AD8232 ad8232;

void sensorPowerMode(bool powerMode) {
  if (powerMode) {
    digitalWrite(ad8232.sdnPin, HIGH);
    ad8232.status = true;
  } else {
    digitalWrite(ad8232.sdnPin, LOW);
    ad8232.status = false;
  }
}

void powerOffLed() {
  digitalWrite(MODE_LED, LOW);
  digitalWrite(ACTIVE_LED, LOW);
  digitalWrite(LOW_BAT_LED, LOW);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
}