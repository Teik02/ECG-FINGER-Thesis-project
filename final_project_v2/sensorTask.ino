  /**
 * @brief 
 * 
 *
 * @return 
 */

extern cppQueue ecgQueue;
extern SimpleKalmanFilter kalmanFilter;
extern ECGDataInfo ecgData;

bool sensorTask(int SENSOR_PIN, uint32_t SAMPLE_RATE, uint32_t NUMBER_OF_SAMPLES, int kalmanMode) {
  const uint32_t SENSOR_UPDATE_INTERVAL = 1000000 / SAMPLE_RATE; // Sensor update interval
  static uint32_t previousMicros = 0;

  uint32_t currentMicros = micros();
  if ((int32_t)(currentMicros - previousMicros) < (int32_t)SENSOR_UPDATE_INTERVAL) {
    return false; // Skip if interval hasn't passed
  }
  previousMicros = currentMicros;

  int rawData = analogRead(SENSOR_PIN);
  updateKalmanFilter((float) rawData, SAMPLE_RATE, kalmanMode);
  float filteredData = kalmanFilter.updateEstimate(rawData); // Apply Kalman filter

  serialTransfer(rawData, seeedXiao.serialFlag);
  serialTransfer(filteredData, seeedXiao.serialFlag);

  ecgData.data.values[ecgData.index] = (uint16_t)filteredData - ecgData.hrCalculate.avgValue;

  if (filteredData < ecgData.hrCalculate.avgValue) {
    ecgData.hrCalculate.updated = true;
  }

  ecgData.data.heartRate = calculateAverageHR((uint16_t) filteredData);


  ecgData.index = (ecgData.index + 1) % NUMBER_OF_SAMPLES;

  // Once we've filled the buffer with NUMBER_OF_SAMPLES readings:
  if (ecgData.index == 0) {
    // Increment the packet counter before pushing
    ecgData.data.packetCounter = (ecgData.data.packetCounter + 1) % 256;

    // Push the completed packet into the queue
    if (!ecgQueue.isFull()) {
      ecgQueue.push(&ecgData.data);
    } else {
      // Serial.println("Queue is full! Data may be lost.");
      // Optional: Handle queue overflow (e.g., discard oldest data or stop sampling until queue has space)
    }

    ecgData.updated = false; // Clear the flag as we've just handled the packet
  }

  return true;
}

void updateKalmanFilter(float rawData, int sampleRate, int setMode) {
  myStats.add(rawData);
  // Cập nhật sai số đo sau khi đủ số mẫu
  if (myStats.count() >= sampleRate * 3) {
    ecgData.hrCalculate.maxPeak = myStats.maximum();
    ecgData.hrCalculate.avgValue = myStats.average();
    myStats.clear();
  }
  changeKalmanMode(setMode);

}

void calculateHR(uint16_t currentValue) {
  if (!ecgData.hrCalculate.updated) {
    return;
  }

  // Điều kiện phát hiện đỉnh
  if (currentValue <= 0.90f * (ecgData.hrCalculate.maxPeak)) {
    return;
  }

  uint32_t currentPeak = micros();

  float heartRate = 60000000.0f / (float)(currentPeak - ecgData.hrCalculate.previousPeak);
  ecgData.hrCalculate.previousPeak = currentPeak;
  // Cập nhật dữ liệu tính trung bình
  if (heartRate < 40 || heartRate > 200) {
    return;
  }
  ecgData.hrCalculate.hrSum += heartRate;
  ecgData.hrCalculate.hrCount++;
  ecgData.hrCalculate.updated = false;
}

uint16_t calculateAverageHR(uint16_t ecgValue) {
  calculateHR(ecgValue);
  static uint32_t lastCalculate = 0;

  if (micros() - lastCalculate < ecgData.hrCalculate.interval) {
    return ecgData.hrCalculate.result;
  }

  if (ecgData.hrCalculate.hrCount > 0) {
    ecgData.hrCalculate.result = ecgData.hrCalculate.hrSum / (float)ecgData.hrCalculate.hrCount;
    // Reset sau khi tính trung bình
    ecgData.hrCalculate.hrSum = 0.0f;
    ecgData.hrCalculate.hrCount = 0;
  }
  lastCalculate = micros();
  return ecgData.hrCalculate.result;
}

void serialTransfer(uint16_t value, bool serialFlag)
{
  if (serialFlag) {
    // Serial.print(rawData);
    // Serial.print(",");
    // Serial.println(data);
    byte highByte = (value >> 8) & 0xFF;
    byte lowByte = value & 0xFF;
    // Serial.println(value);
    Serial.write(highByte); 
    Serial.write(lowByte);
  }
}

  void changeKalmanMode(uint16_t changeMode) {
  switch (changeMode) {
    case 1: {
        float e_mea = myStats.pop_stdev();
        if (e_mea > 0) {
          kalmanFilter.setMeasurementError(e_mea);
          kalmanFilter.setProcessNoise(kalmanSetup.defaultQ);
        }
      } break;
    case 2: {
        kalmanFilter.setMeasurementError(kalmanSetup.manualE_mea);
        kalmanFilter.setProcessNoise(kalmanSetup.manualQ);
      } break;
    default: {
        kalmanFilter.setMeasurementError(kalmanSetup.defaultE_mea);
        kalmanFilter.setProcessNoise(kalmanSetup.defaultQ);
      } break;
  }
  if(!kalmanSetup.flag) {
    return;
  }
  ecgQueue.flush();
  kalmanSetup.flag = false;
  switch (changeMode) {
    case 1: {blinkModeLed1(2);}
      break;
    case 2: {blinkModeLed2(2);}
      break;
    default: {blinkModeLed3(2);}
      break;
  }
}
