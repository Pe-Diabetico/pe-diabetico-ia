/*
 * ============================================================
 *  Monitor Plantar — ESP32 BLE + Random Forest Embarcado
 *  Versão: 2.0.0
 *
 *  Hardware:
 *    FSR Calcâneo  → pino 36 (VP)
 *    FSR Meta 1    → pino 39 (VN)
 *    FSR Meta 5    → pino 33
 *    DHT22 (data)  → pino 27
 *    OLED SDA      → pino 5
 *    OLED SCL      → pino 4
 *
 *  Fluxo:
 *    1. Lê sensores FSR + DHT22
 *    2. Converte ADC → kPa
 *    3. Roda predict() do model.h → risco 0 ou 1
 *    4. Exibe resultado no OLED
 *    5. Envia JSON via BLE com dados + risco
 *
 *  ANTES de compilar:
 *    → Rode: python exportar_modelo_esp32.py
 *    → Copie o model.h gerado para esta pasta
 *
 *  Bibliotecas (Library Manager):
 *    Adafruit SSD1306 | Adafruit GFX | DHT sensor library | ArduinoJson 6.x
 *    BLE já inclusa no core ESP32.
 * ============================================================
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

// ── Modelo Random Forest gerado pelo Python ──────────────────
// Execute exportar_modelo_esp32.py para gerar este arquivo.
// Se ainda não gerou, comente esta linha e o firmware compila
// sem ML — apenas com envio de dados.
#include "model.h"

// ── Configurações de hardware ────────────────────────────────
#define PIN_CALCANEO    36
#define PIN_META1       39
#define PIN_META5       33
#define DHTPIN          27
#define DHTTYPE         DHT22
#define OLED_SDA        5
#define OLED_SCL        4
#define OLED_ADDR       0x3C
#define SCREEN_W        128
#define SCREEN_H        64

// ── Configurações de operação ────────────────────────────────
#define AMOSTRAS_FSR      10
#define INTERVALO_DHT_MS  2100
#define INTERVALO_LOOP_MS 500

// ── BLE ──────────────────────────────────────────────────────
#define BLE_NOME  "MonitorPlantar"
#define SVC_UUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// ── Calibração ADC → kPa ─────────────────────────────────────
// Conversão linear simplificada: 0 ADC = 0 kPa, 4095 ADC = 600 kPa
// Ajuste KPA_MAX com pesos conhecidos após calibração física
#define KPA_MAX 600.0f
float adcParaKpa(int adc) {
  return (adc / 4095.0f) * KPA_MAX;
}

// ── Instâncias globais ───────────────────────────────────────
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE);

BLEServer*         pServer         = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;
bool oldConnected    = false;

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
// Recebidos via BLE quando o profissional conecta o app.
// O app envia esses valores uma vez por sessão no JSON de configuração.
// Valores padrão: medianas populacionais (usados até o app enviar os reais).
float pacIdade        = 65.0f;
float pacIMC          = 28.0f;
float pacAnosDiabetes = 8.0f;
float pacHbA1c        = 7.0f;
float pacHAS          = 0.0f;   // 0 = não, 1 = sim
float pacTabagismo    = 0.0f;   // 0 = não, 1 = sim
bool  dadosClinicos   = false;  // true quando o app enviou os dados reais

// ── Inferência do Random Forest ──────────────────────────────
//
// Features na ordem EXATA definida em exportar_modelo_esp32.py:
//   [ 0] idade
//   [ 1] imc
//   [ 2] anos_diabetes
//   [ 3] hba1c
//   [ 4] has
//   [ 5] tabagismo
//   [ 6] kpa_calcaneo   → adcParaKpa(adcCal)
//   [ 7] kpa_meta1      → adcParaKpa(adcM1)
//   [ 8] kpa_meta5      → adcParaKpa(adcM5)
//   [ 9] temp           → leitura DHT22 (°C)
//   [10] umid           → leitura DHT22 (%)
//
// Fallbacks usados enquanto o DHT22 ainda não leu:
//   temp → 32.0°C (temperatura plantar típica em repouso)
//   umid → 50.0%  (umidade média interior de calçado)
void executarPredicao() {
  float features[11] = {
    pacIdade,
    pacIMC,
    pacAnosDiabetes,
    pacHbA1c,
    pacHAS,
    pacTabagismo,
    adcParaKpa(adcCal),
    adcParaKpa(adcM1),
    adcParaKpa(adcM5),
    isnan(sTemp) ? 32.0f : sTemp,
    isnan(sUmid) ? 50.0f : sUmid,
  };

  // predict() gerado pelo micromlgen — retorna 0=BAIXO ou 1=ALTO
  riscoClasse = predict(features);

  // predictProba() — se o model.h não tiver esta função, comente as 3 linhas
  float proba[2];
  predictProba(features, proba);
  riscoProb = proba[riscoClasse] * 100.0f;
}

// ── Atualiza OLED ────────────────────────────────────────────
void atualizarDisplay() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  // Cabeçalho + status BLE
  display.setCursor(0, 0);
  display.print(F("MONITOR PLANTAR"));
  display.setCursor(104, 0);
  display.println(deviceConnected ? F("BLE:OK") : F("BLE:--"));
  display.drawLine(0, 9, SCREEN_W, 9, WHITE);

  // Pressões
  display.setCursor(0, 11);
  display.printf("Cal:%5.0f  M1:%5.0f", adcParaKpa(adcCal), adcParaKpa(adcM1));
  display.setCursor(0, 20);
  display.printf("M5: %5.0f kPa", adcParaKpa(adcM5));

  // Temperatura e umidade
  display.setCursor(0, 30);
  if (!isnan(sTemp) && !isnan(sUmid)) {
    display.printf("T:%.1fC  U:%.0f%%", sTemp, sUmid);
  } else {
    display.print(F("DHT22: aguardando"));
  }

  display.drawLine(0, 40, SCREEN_W, 40, WHITE);

  // Resultado do modelo — destaque visual
  display.setCursor(0, 43);
  display.print(F("RISCO: "));

  if (riscoClasse < 0) {
    display.print(F("calculando..."));
  } else {
    // Texto maior para o resultado
    display.setTextSize(1);
    if (riscoClasse == 1) {
      display.print(F("*** ALTO ***"));
    } else {
      display.print(F("BAIXO"));
    }
    if (riscoProb > 0) {
      display.setCursor(0, 54);
      display.printf("Confianca: %.0f%%", riscoProb);
    }
  }

  display.display();
}

// ── Envia JSON via BLE ───────────────────────────────────────
// Formato completo enviado ao app:
// {
//   "adc_cal":      2100,
//   "adc_m1":       1820,
//   "adc_m5":       1540,
//   "kpa_calcaneo": "183.5",
//   "kpa_meta1":    "201.4",
//   "kpa_meta5":    "145.8",
//   "temp":         "32.4",   (ou null)
//   "umid":         "61.0",   (ou null)
//   "risco":        1,        (0=baixo / 1=alto / null=calculando)
//   "risco_prob":   "87.3",   (probabilidade em %)
//   "seq":          142
// }
void enviarBLE() {
  StaticJsonDocument<300> doc;

  // ADC brutos (para calibração futura no computador)
  doc["adc_cal"] = adcCal;
  doc["adc_m1"]  = adcM1;
  doc["adc_m5"]  = adcM5;

  // Valores em kPa
  doc["kpa_calcaneo"] = serialized(String(adcParaKpa(adcCal), 1));
  doc["kpa_meta1"]    = serialized(String(adcParaKpa(adcM1),  1));
  doc["kpa_meta5"]    = serialized(String(adcParaKpa(adcM5),  1));

  // Temperatura e umidade
  if (!isnan(sTemp) && !isnan(sUmid)) {
    doc["temp"] = serialized(String(sTemp, 1));
    doc["umid"] = serialized(String(sUmid, 1));
  } else {
    doc["temp"] = nullptr;
    doc["umid"] = nullptr;
  }

  // Resultado do Random Forest
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
  Serial.println(F("[BOOT] Monitor Plantar v2.0 — RF embarcado"));

  Wire.begin(OLED_SDA, OLED_SCL);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("[AVISO] OLED nao encontrado."));
  } else {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(20, 15);
    display.println(F("Monitor Plantar"));
    display.setCursor(20, 28);
    display.println(F("Carregando RF..."));
    display.display();
  }

  // BLE
  BLEDevice::init(BLE_NOME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCB());

  BLEService* svc = pServer->createService(SVC_UUID);

  // Característica NOTIFY — envia dados dos sensores ao app
  pCharacteristic = svc->createCharacteristic(
    CHAR_UUID, BLECharacteristic::PROPERTY_NOTIFY
  );
  pCharacteristic->addDescriptor(new BLE2902());

  // Característica WRITE — recebe dados clínicos do app
  // UUID derivado do CHAR_UUID com último byte alterado
  #define CHAR_WRITE_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
  BLECharacteristic* pCharWrite = svc->createCharacteristic(
    CHAR_WRITE_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );

  // Callback para processar dados clínicos recebidos do app
  class WriteCB : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override {
      String val = pChar->getValue().c_str();
      StaticJsonDocument<256> doc;
      if (deserializeJson(doc, val) == DeserializationError::Ok) {
        // Atualiza apenas os campos presentes no JSON recebido
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

  delay(2000);
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

  // 4. Atualiza OLED
  atualizarDisplay();

  // 5. Envia BLE
  if (deviceConnected) enviarBLE();

  // 6. Reconexão automática
  if (!deviceConnected && oldConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println(F("[BLE] Advertising reiniciado."));
    oldConnected = false;
  }
  if (deviceConnected && !oldConnected) oldConnected = true;

  delay(INTERVALO_LOOP_MS);
}
