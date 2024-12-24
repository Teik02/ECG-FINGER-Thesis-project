/* Add library */

/* Bluetooth low energy library & configuration */
#include <bluefruit.h>
#define BLE_UUID_ECG_SERVICE "180C"
#define BLE_UUID_ECG "2A56"
#define BLE_PACKET_SAMPLES 225
#define BLE_DEVICE_NAME "ECGFINGER"
#define BLE_UPDATE_INTERVAL 500000

/* Variables define */
#define SENSOR_SAMPLE_RATE   200    // Sample per second.
#define BATTERY_INTERVAL      5      // sec.
const int adcResolution = 12;

/* Kalman filter library & configuration */
#include <SimpleKalmanFilter.h>
SimpleKalmanFilter kalmanFilter(0.001, 1, 1);
SimpleKalmanFilter batteryKalmanFilter(1,1,0.001);

/* Statistic library & configuration */
#include "Statistic.h"
statistic::Statistic<float, uint32_t, true> myStats;

/* Queue library & configuration */
#include <cppQueue.h>

struct SeeedXiao {
  bool sleepFlag;
  bool sleepFlagChange;
  bool serialFlag;
  bool verryLowBattery;
};

SeeedXiao seeedXiao = {
  .sleepFlag = false,
  .sleepFlagChange = false,
  .serialFlag = false,
  .verryLowBattery = false
};

struct BATTERY {
  const int enablePin;
  const int readPin;
  const bool fastCharge;
  const bool normalCharge;
  uint16_t batPer;
  const float vRef;
  float volt;
  const uint32_t intervalTime;
  const int HICHG;
};

BATTERY battery = {
  .enablePin = 14,
  .readPin = 32,
  .fastCharge = true,
  .normalCharge = false,
  .batPer = 0,
  .vRef = 3.26f,
  .volt = 0.0f,
  .intervalTime = 1000*BATTERY_INTERVAL
};

struct AD8232 {
  int sensorPin;   // Pin cảm biến ECG (AD8232 output)
  int sdnPin;      // Pin điều khiển Shutdown (SDN)
  int lodPosPin;   // Pin phát hiện Leads Off Detection (dương)
  int lodNegPin;   // Pin phát hiện Leads Off Detection (âm)
  uint16_t sampleRate;
  bool turnOn;
  bool turnOff;
  bool status;
};

// Khởi tạo cấu hình cảm biến
AD8232 ad8232 = {
  .sensorPin = A0,     // AD8232 output pin
  .sdnPin = D2,        // Shutdown control pin
  .lodPosPin = D3,     // Leads Off Detection positive pin
  .lodNegPin = D4,      // Leads Off Detection negative pin
  .sampleRate = SENSOR_SAMPLE_RATE,
  .turnOn = true,
  .turnOff = false,
  .status = false
};


struct KalmanSetup {
  float manualE_mea;
  float e_est;
  float manualQ;
  float defaultQ;
  float defaultE_mea;
  bool flag;
  bool update;
};

KalmanSetup kalmanSetup = {
  .manualE_mea = 1,
  .e_est = 1,
  .manualQ = 0.01,
  .defaultQ = 1,
  .defaultE_mea = 0.001,
  .flag = true,
  .update = false
};

/* LED configuration */
#define MODE_LED D10
#define ACTIVE_LED   D9 
#define LOW_BAT_LED  D8
const int CHG_BAT_LED = 23;

/* BUTTON configuration */
#define MODE_BUTTON D7
const uint32_t LONG_PRESS_DURATION = 500;
bool buttonPressed = false;
uint32_t pressTime = 0;
int mode = 3;



// ECG Data Structure
union ECGData {
  struct __attribute__((packed)) {
    int16_t values[BLE_PACKET_SAMPLES];
    uint16_t heartRate;
    uint8_t packetCounter;
  };
  uint8_t bytes[BLE_PACKET_SAMPLES * sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint8_t)];
};

struct HeartRateCalculate {
  float hrSum = 0.0f;
  int hrCount = 0;
  uint32_t previousPeak = 0;
  bool updated = true;
  float avgValue = 0.0f;
  float maxPeak = 0.0f;
  const uint32_t interval = 2000000;
  uint16_t result = 0;
};

struct ECGDataInfo {
  ECGData data;
  HeartRateCalculate hrCalculate;
  uint32_t index = 0;
  bool updated = false;
};
ECGDataInfo ecgData;


// BLEBas blebas;
BLEService ecgService(BLE_UUID_ECG_SERVICE);
BLECharacteristic ecgCharacteristic(BLE_UUID_ECG, BLERead | BLENotify, sizeof(ecgData.data.bytes));
uint16_t g_conn_handle;
bool bleConnectedFlag = false;

// Create a FIFO queue for ECGData packets
cppQueue ecgQueue(sizeof(ECGData), 10, FIFO);


// uint32_t interruptCount = 0;
// uint32_t lastInterruptTime = 0;

void setup() {
  Serial.begin(115200);
  delay(100);
  initInterruptButton();
  initPin();
  initADC(ADC_RESOLUTION);
  initBLE();
  // sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
  sensorPowerMode(ad8232.turnOff);
  powerOffLed();
  blinkLed(ACTIVE_LED, 250, 4);

  ecgData.data.packetCounter = 0;
  
  Bluefruit.disconnect(g_conn_handle);
  // Serial.println("Hello, I am awake!");
}

void loop() {
  // if(Serial) {
  //   if (!seeedXiao.sleepFlag) {
  //     sensorPowerMode(ad8232.turnOn);
  //     if (!sensorTask(ad8232.sensorPin, ad8232.sampleRate, BLE_PACKET_SAMPLES, mode)) {

  //     }
  //   }
  // } else {
  //   if (!buttonMode()) {
  //     return;
  //   }

  if(Serial || !digitalRead(CHG_BAT_LED)) {
    seeedXiao.serialFlag = true;
    seeedXiao.verryLowBattery = false;
  } else {
    seeedXiao.serialFlag = false;
  }

  // if (seeedXiao.verryLowBattery) {
  //   seeedXiao.verryLowBattery;
  // }
  
  if (!buttonMode()) {
    return;
  }
  readBattery();
  if (!setBatteryLedMode(seeedXiao.serialFlag)) {
    Bluefruit.Advertising.stop();
    Bluefruit.disconnect(g_conn_handle);
    // if (!Serial) {
    //   sensorPowerMode(ad8232.turnOff);
    //   return;
    // } 

    // Vì if sẽ thực hiện nếu pin thấp (do setBatteryLedMode sẽ trả về false nếu pin thấp) nên cần cờ để return, tránh trường hợp cảm biến hoạt động khi pin thấp.
    if (seeedXiao.verryLowBattery) {
      return;
    }

    sensorPowerMode(ad8232.turnOn);
    if (!sensorTask(ad8232.sensorPin, ad8232.sampleRate, BLE_PACKET_SAMPLES, mode)) {

    }
    return;
  }
  if (!bleConnectedFlag) {
    Bluefruit.Advertising.start(0);
  }
  // if (!ad8232.status) {
  //   return;
  // }
  if (!sensorTask(ad8232.sensorPin, ad8232.sampleRate, BLE_PACKET_SAMPLES, mode)) {
    bleTask(BLE_UPDATE_INTERVAL);
  }
  
}




