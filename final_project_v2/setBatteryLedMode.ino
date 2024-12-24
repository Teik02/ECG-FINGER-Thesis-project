/**
 * @brief Sets the battery LED mode based on the battery percentage.
 *
 * @param percent The current battery percentage.
 */

extern BATTERY battery;

bool setBatteryLedMode(bool flag) {
  const int VERY_LOW_BATTERY_THRESHOLD = 5;
  const int LOW_BATTERY_THRESHOLD = 20;
  const int CHG_THRESHOLH = 95;

  // setChargingMode(battery.normalCharge);

  if (flag) {
    // sensorPowerMode(ad8232.turnOff);
    if (battery.batPer > CHG_THRESHOLH) {
      // Turn off battery status LEDs
      // blinkLed(ACTIVE_LED, 500, 2);
      digitalWrite(LOW_BAT_LED, LOW);
      digitalWrite(ACTIVE_LED, HIGH);
      // Turn off sensor power and set charging mode
      // setChargingMode(NORMAL_CHARGE);
      // Exit the function early
    // } else if (battery.batPer > LOW_BATTERY_THRESHOLD){
    //   digitalWrite(ACTIVE_LED, LOW);
    //   digitalWrite(LOW_BAT_LED, HIGH);

    } else {
      digitalWrite(ACTIVE_LED, LOW);
      digitalWrite(LOW_BAT_LED, HIGH);
      // blinkLed(LOW_BAT_LED, 500, 2);
    }
    return false;
  } else {
    if (battery.batPer > LOW_BATTERY_THRESHOLD) {
      // Battery is above 20%, turn on active LED and turn off low battery LED
      digitalWrite(LOW_BAT_LED, LOW);
      digitalWrite(ACTIVE_LED, HIGH);
    } else if (battery.batPer > VERY_LOW_BATTERY_THRESHOLD) {
      // Battery is between 10% and 20%, turn on low battery LED and turn off active battery LED
      digitalWrite(ACTIVE_LED, LOW);
      digitalWrite(LOW_BAT_LED, HIGH);
    } else {
      // Battery is below 10%, turn off sensor power and blink low battery LED
      // sensorPowerMode(ad8232.turnOff);
      // blinkLed(LOW_BAT_LED, 200, 5);
      seeedXiao.sleepFlag = true;
      seeedXiao.verryLowBattery = true;
      enterDeepSleep();
      return false;
    }
    return true;
  }

}