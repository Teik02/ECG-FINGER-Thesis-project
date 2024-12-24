/**
 * @brief Blinks the low battery LED.
 */
void blinkLed(int ledPin, int blinkDelay, int blinkTime) {
  for (int i = 0; i < blinkTime; i++) {
    digitalWrite(ledPin, HIGH);
    delay(blinkDelay);
    digitalWrite(ledPin, LOW);
    delay(blinkDelay);
  }
}

void blinkModeLed1(uint16_t time) {
  digitalWrite(MODE_LED, LOW);
  digitalWrite(LOW_BAT_LED, LOW);
  blinkLed(ACTIVE_LED, 150, time);
}

void blinkModeLed2(uint16_t time) {
  digitalWrite(ACTIVE_LED, LOW);
  digitalWrite(LOW_BAT_LED, LOW);
  blinkLed(MODE_LED, 150, time);
}

void blinkModeLed3(uint16_t time) {
  digitalWrite(ACTIVE_LED, LOW);
  digitalWrite(MODE_LED, LOW);
  blinkLed(LOW_BAT_LED, 150, time);
}

void blinkDisconnectLed(uint16_t time) {
  digitalWrite(ACTIVE_LED, LOW);
  digitalWrite(MODE_LED, LOW);
  blinkLed(LOW_BAT_LED, 150, time);
}