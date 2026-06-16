/*
 * Monitor Plantar - Sistema Integrado (FSRs + AHT20 + BLE)
 * Hardware: D32, Resistores 10k/20k, I2C 21/22
 */

#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ── Hardware ─────────────────────────────────────────────
#define PINO_LED 5
#define PIN_CALCANEO 36
#define PIN_META1    39
#define PIN_META5    33

// ── Config BLE ───────────────────────────────────────────
#define BLE_NOME  "MonitorPlantar"
#define SVC_UUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

Adafruit_AHTX0 aht;
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
uint32_t seqBLE = 0;

// Variáveis de Suavização (Pressão)
float calSuave = 0, m1Suave = 0, m5Suave = 0;
const float alfa = 0.3;

float converterParaKpa(int adc) { return (adc / 4095.0f) * 600.0f; }

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
  Wire.begin(21, 22);

  // Inicializa AHT20
  if (!aht.begin(&Wire)) Serial.println(F("ERRO: AHT20 não encontrado."));

  // Inicializa BLE
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
  // 1. Leitura sensores
  sensors_event_t umid, temp;
  aht.getEvent(&umid, &temp);

  float kpaCal = converterParaKpa(analogRead(PIN_CALCANEO));
  float kpaM1  = converterParaKpa(analogRead(PIN_META1));
  float kpaM5  = converterParaKpa(analogRead(PIN_META5));

  // 2. Filtro
  calSuave = (alfa * kpaCal) + ((1.0 - alfa) * calSuave);
  m1Suave  = (alfa * kpaM1)  + ((1.0 - alfa) * m1Suave);
  m5Suave  = (alfa * kpaM5)  + ((1.0 - alfa) * m5Suave);

  if (deviceConnected) {
    digitalWrite(PINO_LED, HIGH);

    // 3. JSON Integrado
    StaticJsonDocument<256> doc;
    doc["temp"] = temp.temperature;
    doc["umid"] = umid.relative_humidity;
    doc["kpa_calcaneo"] = (float)calSuave;
    doc["kpa_meta1"]    = (float)m1Suave;
    doc["kpa_meta5"]    = (float)m5Suave;
    doc["seq"]          = seqBLE++;

    char buf[256];
    serializeJson(doc, buf);
    pCharacteristic->setValue(buf);
    pCharacteristic->notify();

    // Feedback visual de envio
    digitalWrite(PINO_LED, LOW); delay(5); digitalWrite(PINO_LED, HIGH);
    delay(200);
  } else {
    // LED modo busca
    digitalWrite(PINO_LED, (millis() / 500) % 2);
    delay(200);
  }
}