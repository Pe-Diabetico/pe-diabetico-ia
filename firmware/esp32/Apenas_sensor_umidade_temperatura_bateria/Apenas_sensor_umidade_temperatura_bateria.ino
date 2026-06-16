/*
 * MonitorPlantar - Sistema Integrado com Sensor AHT e BLE
 * Correção: Pinos I2C 21/22, Brownout desativado, Feedback visual em LED.
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

// ── Definições de Hardware ────────────────────────────────────
#define PINO_LED 5 // LED integrado no pino 5 conforme esquema D32[cite: 1]

// ── Configurações BLE ──────────────────────────────────────────
#define BLE_NOME  "MonitorPlantar"
#define SVC_UUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// ── Variáveis Globais ──────────────────────────────────────────
Adafruit_AHTX0 aht;
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
unsigned long tempoPiscar = 0;
bool estadoLed = false;
uint32_t seqBLE = 0;

// ── Callbacks de Conexão BLE ───────────────────────────────────
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) { deviceConnected = true; };
    void onDisconnect(BLEServer* pServer) { deviceConnected = false; }
};

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  pinMode(PINO_LED, OUTPUT);
  Serial.begin(115200);
  delay(2000);

  // Piscar inicial
  for(int i=0; i<3; i++) {
    digitalWrite(PINO_LED, HIGH); delay(150);
    digitalWrite(PINO_LED, LOW); delay(150);
  }

  Serial.println(F("\n[BOOT] Iniciando Sistema..."));

  Wire.begin(21, 22);
  
  // NÃO TRAVAR MAIS O CÓDIGO SE O SENSOR FALHAR
  if (!aht.begin(&Wire)) {
    Serial.println(F("ERRO: Sensor não detectado. Iniciando Bluetooth mesmo assim..."));
  } else {
    Serial.println(F("[OK] Sensor AHT detectado!"));
  }

  // Inicializa Bluetooth
  BLEDevice::init(BLE_NOME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SVC_UUID);
  pCharacteristic = pService->createCharacteristic(CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SVC_UUID);
  pAdvertising->start();

  Serial.println(F("[BLE] Iniciado! Aguardando conexao..."));
}

void loop() {
  if (deviceConnected) {
    // Modo Conectado: LED fixo com micropulso de transmissão
    digitalWrite(PINO_LED, HIGH);
    
    sensors_event_t umidade, temp;
    aht.getEvent(&umidade, &temp);

    StaticJsonDocument<200> doc;
    doc["temp"] = serialized(String(temp.temperature, 1));
    doc["umid"] = serialized(String(umidade.relative_humidity, 1));
    doc["seq"]  = seqBLE++;

    char jsonString[200];
    serializeJson(doc, jsonString);
    pCharacteristic->setValue(jsonString);
    pCharacteristic->notify();

    // Efeito visual de envio: pisca rápido (micropulso)
    digitalWrite(PINO_LED, LOW); delay(10); digitalWrite(PINO_LED, HIGH);
    delay(2000); 
  } else {
    // Modo Busca: piscar compassado (anunciando BLE)
    if (millis() - tempoPiscar > 500) {
      estadoLed = !estadoLed;
      digitalWrite(PINO_LED, estadoLed);
      tempoPiscar = millis();
    }
  }
}