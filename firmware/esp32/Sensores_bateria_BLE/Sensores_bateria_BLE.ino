/*
 * Monitor Plantar - FSRs + BLE + Conversão para kPa (Opção B)
 * O ESP32 envia kPa real, pronto para exibição no App.
 */

#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define PINO_LED 5 
#define PIN_CALCANEO 36
#define PIN_META1    39
#define PIN_META5    33

// BLE Config
#define BLE_NOME  "MonitorPlantar"
#define SVC_UUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
uint32_t seqBLE = 0;

// Variáveis de Suavização
float calSuave = 0, m1Suave = 0, m5Suave = 0;
const float alfa = 0.3; 

// Função de conversão ADC (0-4095) para kPa (0-600)
float converterParaKpa(int adc) {
  return (adc / 4095.0f) * 600.0f;
}

class ServerCB : public BLEServerCallbacks {
  void onConnect(BLEServer*)    override { deviceConnected = true; }
  void onDisconnect(BLEServer*) override { deviceConnected = false; }
};

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  pinMode(PINO_LED, OUTPUT);
  Serial.begin(115200);
  
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  BLEDevice::init(BLE_NOME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCB());
  BLEService* svc = pServer->createService(SVC_UUID);
  pCharacteristic = svc->createCharacteristic(CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  svc->start();
  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SVC_UUID);
  adv->start();
}

void loop() {
  // 1. Lê valor bruto e converte para kPa imediatamente
  float kpaCal = converterParaKpa(analogRead(PIN_CALCANEO));
  float kpaM1  = converterParaKpa(analogRead(PIN_META1));
  float kpaM5  = converterParaKpa(analogRead(PIN_META5));

  // 2. Aplica Filtro de Média Móvel
  calSuave = (alfa * kpaCal) + ((1.0 - alfa) * calSuave);
  m1Suave  = (alfa * kpaM1)  + ((1.0 - alfa) * m1Suave);
  m5Suave  = (alfa * kpaM5)  + ((1.0 - alfa) * m5Suave);

  // 3. Serial Monitor (Agora exibe kPa real)
  Serial.printf("KPA REAL -> Cal:%.1f | M1:%.1f | M5:%.1f\n", calSuave, m1Suave, m5Suave);

  // 4. BLE Notification
  if (deviceConnected) {
    digitalWrite(PINO_LED, HIGH);
    
    StaticJsonDocument<200> doc;
    // Enviamos o valor já em kPa para o App
    doc["kpa_calcaneo"] = (float)calSuave;
    doc["kpa_meta1"]    = (float)m1Suave;
    doc["kpa_meta5"]    = (float)m5Suave;
    doc["seq"]          = seqBLE++;

    char buf[200];
    serializeJson(doc, buf);
    pCharacteristic->setValue(buf);
    pCharacteristic->notify();
    
    digitalWrite(PINO_LED, LOW); delay(2); digitalWrite(PINO_LED, HIGH);
    delay(200); // Ajuste para 200ms para maior estabilidade
  } else {
    digitalWrite(PINO_LED, (millis() / 500) % 2);
    delay(200);
  }
}