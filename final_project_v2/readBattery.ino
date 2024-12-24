/**
 * @brief Reads the battery voltage and converts it to a percentage.
 *
 * @return The battery percentage (0-100%).
 */

extern BATTERY battery;

void readBattery() {
  static uint32_t previousTime = 0;
  uint32_t currentTime = millis();
  if (currentTime - previousTime < battery.intervalTime) {
    digitalWrite(battery.enablePin, HIGH);
    return;
  }

  digitalWrite(battery.enablePin, LOW);
  
  uint16_t adcVbat = analogRead(battery.readPin);
  battery.volt = batteryKalmanFilter.updateEstimate(adcVbat);
  battery.volt = (float) (adcVbat * battery.vRef) / (pow(2, ADC_RESOLUTION) -1);

  battery.volt = battery.volt * 3600.0 / 1100.0;

  battery.batPer = map(battery.volt * 100, 320, 410, 0, 100); // Assuming 3.0V = 0% and 4.2V = 100%
  battery.batPer = constrain(battery.batPer, 0, 100); // Ensure the percentage is within 0-100%
  
  // Serial.println(battery.volt);
  // Serial.print(", ");
  previousTime = currentTime;
}