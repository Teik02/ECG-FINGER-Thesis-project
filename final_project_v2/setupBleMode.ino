bool setupBleMode() {
  if (!Bluefruit.begin()) {
    return false; // Failed to initialize BLE
  }

  Bluefruit.setName(BLE_DEVICE_NAME);
  Bluefruit.Periph.setConnectCallback(blePeripheralConnectHandler);
  Bluefruit.Periph.setDisconnectCallback(blePeripheralDisconnectHandler);

  ecgService.begin();
  ecgCharacteristic.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  ecgCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  ecgCharacteristic.setFixedLen(sizeof(ecgData.data.bytes));
  ecgCharacteristic.begin();

  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244); 
  Bluefruit.Advertising.setFastTimeout(30); 
  // Bluefruit.Advertising.start(0);

  return true;
}

void blePeripheralConnectHandler(uint16_t conn_handle) {
  // Serial.print(F("Connected to central: "));
  // Serial.println(conn_handle);
  g_conn_handle = conn_handle;
  bleConnectedFlag = true;
  Bluefruit.Advertising.stop();
  sensorPowerMode(ad8232.turnOn);
  digitalWrite(ACTIVE_LED, LOW);
  digitalWrite(LOW_BAT_LED, LOW);
  // blinkLed(ACTIVE_LED, 200, 4);
  blinkModeLed2(4);
}

void blePeripheralDisconnectHandler(uint16_t conn_handle, uint8_t reason) {
  // Serial.print(F("Disconnected from central: "));
  // Serial.println(conn_handle);
  bleConnectedFlag = false;
  ecgData.data.packetCounter = 0;
  // digitalWrite(ACTIVE_LED, LOW);
  // digitalWrite(LOW_BAT_LED, LOW);
  // digitalWrite(MODE_LED, LOW);
  sensorPowerMode(ad8232.turnOff);
  blinkDisconnectLed(4);
}