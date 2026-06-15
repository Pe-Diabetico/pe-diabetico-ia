/*
 * ============================================================
 *  Monitor Plantar — ESP32 BLE + Random Forest Embarcado
 *  Versão: 2.1.0 (Sem OLED / Com Feedback Visual de LED)
 * ============================================================
 */

#include <Wire.h>
#include <DHT.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

// ── Modelo Random Forest gerado pelo Python ──────────────────
#include "model.h"

// Instância do classificador
Eloquent::ML::Port::RandomForest classificadorRF;

// ── Configurações de hardware ────────────────────────────────
#define PIN_CALCANEO    36
#define PIN_META1       39
#define PIN_META5       33
#define DHTPIN          27
#define DHTTYPE         DHT22
#define LED_PIN         5   // LED Azul integrado na placa

// ── Configurações de operação ────────────────────────────────
#define AMOSTRAS_FSR      10
#define INTERVALO_DHT_MS  2100
#define INTERVALO_LOOP_MS 500

// ── BLE ──────────────────────────────────────────────────────
#define BLE_NOME  "MonitorPlantar"
#define SVC_UUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// ── Calibração ADC → kPa ─────────────────────────────────────
#define KPA_MAX 600.0f
float adcParaKpa(int adc) {
  return (adc / 4095.0f) * KPA_MAX;
}

// ── Instâncias e Variáveis Globais ───────────────────────────
DHT dht(DHTPIN, DHTTYPE);

BLEServer*         pServer         = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
bool oldConnected    = false;
bool estadoLED       = false; // Variável para controlar o piscar do LED

// ── Estado dos sensores ──────────────────────────────────────
int   adcCal = 0, adcM1 = 0, adcM5 = 0;
float sTemp  = NAN, sUmid = NAN;
unsigned long tDHT = 0;

// ── Resultado do modelo ──────────────────────────────────────
int    riscoClasse = -1;   // -1 = sem predição ainda
float  riscoProb   = 0.0f; // probabilidade em %
uint32_t seqBLE    = 0;

// ── BLE callbacks ────────────────────────────────────────────
class ServerCB : public BLEServerCallbacks {
  void onConnect(BLEServer*)    override { deviceConnected = true;  }
  void onDisconnect(BLEServer*) override { deviceConnected = false; }
};

// ── Leitura FSR com média ────────────────────────────────────
int lerFSR(int pino) {
  long soma = 0;
  for (int i = 0; i < AMOSTRAS_FSR; i++) {
    soma += analogRead(pino);
    delay(2);
  }
  return (int)(soma / AMOSTRAS_FSR);
}

// ── Dados clínicos do paciente ───────────────────────────────
float pacIdade        = 65.0f;
float pacIMC          = 28.0f;
float pacAnosDiabetes = 8.0f;
float pacHbA1c        = 7.0f;
float pacHAS          = 0.0f;   
float pacTabagismo    = 0.0f;   
bool  dadosClinicos   = false;  

// ── Inferência do Random Forest ──────────────────────────────
void executarPredicao() {
  float features[11] = {
    pacIdade, pacIMC, pacAnosDiabetes, pacHbA1c, pacHAS, pacTabagismo,
    adcParaKpa(adcCal), adcParaKpa(adcM1), adcParaKpa(adcM5),
    isnan(sTemp) ? 32.0f : sTemp,
    isnan(sUmid) ? 50.0f : sUmid
  };

  riscoClasse = classificadorRF.predict(features);
  
  if (riscoClasse == 1) {
    riscoProb = 85.0f; 
  } else {
    riscoProb = 15.0f; 
  }
}

// ── Envia JSON via BLE ───────────────────────────────────────
void enviarBLE() {
  StaticJsonDocument<300> doc;

  doc["adc_cal"] = adcCal;
  doc["adc_m1"]  = adcM1;
  doc["adc_m5"]  = adcM5;

  doc["kpa_calcaneo"] = serialized(String(adcParaKpa(adcCal), 1));
  doc["kpa_meta1"]    = serialized(String(adcParaKpa(adcM1),  1));
  doc["kpa_meta5"]    = serialized(String(adcParaKpa(adcM5),  1));

  if (!isnan(sTemp) && !isnan(sUmid)) {
    doc["temp"] = serialized(String(sTemp, 1));
    doc["umid"] = serialized(String(sUmid, 1));
  } else {
    doc["temp"] = nullptr;
    doc["umid"] = nullptr;
  }

  if (riscoClasse >= 0) {
    doc["risco"]      = riscoClasse;
    doc["risco_prob"] = serialized(String(riscoProb, 1));
  } else {
    doc["risco"]      = nullptr;
    doc["risco_prob"] = nullptr;
  }

  doc["seq"] = seqBLE++;

  char buf[300];
  serializeJson(doc, buf);
  pCharacteristic->setValue(buf);
  pCharacteristic->notify();
  Serial.println(buf);
}

// ── setup ────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  
  // Configura LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println(F("[BOOT] Monitor Plantar v2.0 — Iniciando..."));

  // Efeito Visual de Boot (Pisca 3 vezes rápido)
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH); delay(150);
    digitalWrite(LED_PIN, LOW);  delay(150);
  }

  dht.begin();

  // BLE Inicialização
  BLEDevice::init(BLE_NOME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCB());

  BLEService* svc = pServer->createService(SVC_UUID);

  pCharacteristic = svc->createCharacteristic(
    CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());

  #define CHAR_WRITE_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
  BLECharacteristic* pCharWrite = svc->createCharacteristic(
    CHAR_WRITE_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );

  class WriteCB : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override {
      String val = pChar->getValue().c_str();
      StaticJsonDocument<256> doc;
      if (deserializeJson(doc, val) == DeserializationError::Ok) {
        if (doc.containsKey("idade"))         pacIdade        = doc["idade"];
        if (doc.containsKey("imc"))           pacIMC          = doc["imc"];
        if (doc.containsKey("anos_diabetes")) pacAnosDiabetes = doc["anos_diabetes"];
        if (doc.containsKey("hba1c"))         pacHbA1c        = doc["hba1c"];
        if (doc.containsKey("has"))           pacHAS          = doc["has"];
        if (doc.containsKey("tabagismo"))     pacTabagismo    = doc["tabagismo"];
        dadosClinicos = true;
        Serial.println(F("[RF] Dados clinicos atualizados via BLE."));
      }
    }
  };
  pCharWrite->setCallbacks(new WriteCB());

  svc->start();

  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SVC_UUID);
  adv->setScanResponse(true);
  adv->setMinPreferred(0x06);
  BLEDevice::startAdvertising();

  Serial.println(F("[BLE] Aguardando conexao..."));
  Serial.println(F("[RF]  Modelo pronto com 11 features."));

  delay(1000);
  tDHT = millis();
}

// ── loop ─────────────────────────────────────────────────────
void loop() {

  // 1. Lê FSR
  adcCal = lerFSR(PIN_CALCANEO);
  adcM1  = lerFSR(PIN_META1);
  adcM5  = lerFSR(PIN_META5);

  // 2. Lê DHT22 no intervalo correto
  if (millis() - tDHT >= INTERVALO_DHT_MS) {
    float t = dht.readTemperature();
    float u = dht.readHumidity();
    if (!isnan(t)) sTemp = t;
    if (!isnan(u)) sUmid = u;
    tDHT = millis();
  }

  // 3. Roda o Random Forest
  executarPredicao();

  // 4. Lógica de BLE e LED Indicativo
  if (deviceConnected) {
    // Mantém o LED aceso fixo indicando conexão...
    digitalWrite(LED_PIN, HIGH); 
    enviarBLE();
    
    // ... e dá um micropisco para baixo provando que o pacote foi enviado!
    digitalWrite(LED_PIN, LOW);
    delay(30);
    digitalWrite(LED_PIN, HIGH);
    
  } else {
    // Se estiver desconectado (Advertising), o LED pisca compassadamente
    estadoLED = !estadoLED;
    digitalWrite(LED_PIN, estadoLED);
  }

  // 5. Reconexão automática
  if (!deviceConnected && oldConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println(F("[BLE] Advertising reiniciado."));
    oldConnected = false;
  }
  if (deviceConnected && !oldConnected) oldConnected = true;

  // Aguarda o intervalo antes de repetir o ciclo
  delay(INTERVALO_LOOP_MS);
}