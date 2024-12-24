// bool enterDeepSleep() {
//   if (!seeedXiao.sleepFlag) {
//     seeedXiao.sleepFlagChange = false;
//     Serial.println("Không ngủ!!!");
//     return true;
//   }
//   Serial.println("Ngủ ngủ ngủ!!!");
//   // sensorPowerMode(ad8232.turnOff);
//   // digitalWrite(BLE_LED, LOW);
//   // digitalWrite(LOW_BAT_LED, LOW);
//   // digitalWrite(ACTIVE_LED, LOW);
//   // Bluefruit.Advertising.stop();
//   // Serial.println("Sleep");
//   return false;
// }

// bool buttonPressed = false;

// void handleButtonInterrupt() {
//   // Ngắt khi có thay đổi trạng thái nút
//   if (digitalRead(MODE_BUTTON) == LOW) {
//     buttonPressed = true;
//     pressTime = millis();
//   } else {
//     buttonPressed = false;
//   }
// }

// bool buttonMode() {
//   static uint32_t lastPressTime = 0;

//   // Xử lý khi nút được nhấn hoặc nhả
//   if (!buttonPressed && (millis() - pressTime) > 50 && pressTime != lastPressTime) { // Debounce
//     uint32_t currentTime = millis();
//     if ((currentTime - pressTime) > LONG_PRESS_DURATION) {
//       if (!seeedXiao.sleepFlagChange) {
//         seeedXiao.sleepFlag = !seeedXiao.sleepFlag;
//         Serial.println(seeedXiao.sleepFlag);
//         seeedXiao.sleepFlagChange = true;
//       }
//       return enterDeepSleep();
//     } else {
//       kalmanDefault.flag = true;
//       kalmanDefault.update = !kalmanDefault.update;
//       Serial.println(kalmanDefault.update);
//     }
//     lastPressTime = pressTime; // Cập nhật thời gian xử lý cuối cùng
//     return true;
//   }
//   return true;
// }

bool enterDeepSleep() {
  if (!seeedXiao.sleepFlag) {
    return false;
  }
  sensorPowerMode(ad8232.turnOff);
  Bluefruit.Advertising.stop();
  Bluefruit.disconnect(g_conn_handle);
  powerOffLed();
  // sd_app_evt_wait();
  return true;
}



void handleButtonInterrupt() {
  if (digitalRead(MODE_BUTTON) == LOW) {
    buttonPressed = true;
    pressTime = millis();
  } else {
    buttonPressed = false;
  }
}

void wakeUp() {
  mode = 3;
  // Serial.println("Wake up, mode 0");
}

void changeMode() {
  mode = (mode % 3) + 1;
  // Serial.print("Mode: ");
  // Serial.println(mode);
}

bool buttonMode() {
  static uint32_t lastPressTime = 0;
  if (!buttonPressed && (millis() - pressTime) > 20 && pressTime != lastPressTime) {
    uint32_t currentTime = millis();
    if ((currentTime - pressTime) > LONG_PRESS_DURATION) {
      if (seeedXiao.serialFlag) {
        return true;
      }
      seeedXiao.sleepFlag = true;
      // Serial.println("Entering deep sleep...");
      bool enterSuccess = enterDeepSleep();
      mode = 0;
      return !enterSuccess;
    } else {
      if (mode == 0) {
        wakeUp();
        Bluefruit.Advertising.stop();
        Bluefruit.disconnect(g_conn_handle);
      } else {
        kalmanSetup.flag = true;
        changeMode();
      }
    }
    lastPressTime = pressTime;
    return true;
  }
  return true;
}




