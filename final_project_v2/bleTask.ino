extern BLEService ecgService;
extern BLECharacteristic ecgCharacteristic;
extern cppQueue ecgQueue;

void bleTask(const uint32_t UPDATE_INTERVAL) {
  static bool isAdvertising = false;
  static uint32_t previousMillis = 0;

  uint32_t currentMillis = micros();
  if (currentMillis - previousMillis >= UPDATE_INTERVAL) {
    previousMillis = currentMillis;

    // if (!isAdvertising) {
    //   Bluefruit.Advertising.start();
    //   isAdvertising = true;
    // }
  }

  // If there is data in the queue, send it out
  if (!ecgQueue.isEmpty()) {
    ECGData queuedData;
    ecgQueue.pop(&queuedData);
    ecgCharacteristic.notify(queuedData.bytes, sizeof(queuedData.bytes));
  }
}