extern AD8232 ad8232;

/**
 * @brief Initializes the pins for the AD8232 sensor.
 * 
 * This function sets up the necessary pins for the AD8232 sensor. It configures the shutdown pin (SDN) as an output
 * and the lead-off detection pins (LOD_POS and LOD_NEG) as inputs.
 *
 * @return void
 */
void initPin() {
  /* AD8232 pin */
  pinMode(ad8232.sdnPin, OUTPUT);
  pinMode(ad8232.lodPosPin, INPUT);
  pinMode(ad8232.lodNegPin, INPUT);

  /* Led pin */
  pinMode(ACTIVE_LED, OUTPUT);
  pinMode(LOW_BAT_LED, OUTPUT);
  pinMode(MODE_LED, OUTPUT);
  pinMode(CHG_BAT_LED, INPUT);

  /* Battery pin */
  // pinMode(HICHG, OUTPUT);
  // pinMode(BAT_ENABLE_READ, OUTPUT);

}

/**
* @brief
*
* @param
*
* @return
*/
void initADC(int adcRes) {
  analogReadResolution(adcRes);
}

/**
* @brief
*
* @param
*
* @return
*/
void initBLE() {
  // Configure maximum bandwidth
  Bluefruit.configCentralBandwidth(BANDWIDTH_MAX);
  Bluefruit.setTxPower(8);

  if (!setupBleMode()) {
    // Serial.println("Failed to initialize BLE!");
    while (1);
  } else {
    // Serial.println("BLE initialized. Waiting for clients to connect.");
  }
}

void initInterruptButton() {

  pinMode(MODE_BUTTON, INPUT_PULLUP); // Cấu hình pin cho nút nhấn
  attachInterrupt(digitalPinToInterrupt(MODE_BUTTON), handleButtonInterrupt, CHANGE);
}

