/*
 * ============================================================
 *  Monitor Plantar — ESP32 BLE
 *  Sensores: 3× FSR (pressão), DHT22 (temp + umidade)
 *  Display:  OLED SSD1306 128×64 via I²C (pinos SDA=5, SCL=4)
 *  BLE:      envia JSON a cada 500 ms via NOTIFY
 *
 *  Bibliotecas necessárias (Library Manager):
 *    - Adafruit SSD1306
 *    - Adafruit GFX Library
 *    - DHT sensor library (Adafruit)
 *    - ArduinoJson  (versão 6.x)
 *
 *  A biblioteca BLE já vem no core ESP32 para Arduino.
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

// ── Display OLED ─────────────────────────────────────────────
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ── Pinos FSR ────────────────────────────────────────────────
const int PIN_CALCANEO = 36;   // VP  — apenas entrada analógica
const int PIN_META1    = 39;   // VN  — apenas entrada analógica
const int PIN_META5    = 33;

// ── DHT22 ────────────────────────────────────────────────────
#define DHTPIN  27
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ── Média de amostras FSR ────────────────────────────────────
const int NUM_AMOSTRAS = 10;

int lerSensorMedia(int pino) {
  int soma = 0;
  for (int i = 0; i < NUM_AMOSTRAS; i++) {
    soma += analogRead(pino);
    delay(2);   // respeita tempo de conversão do ADC 12 bits
  }
  return soma / NUM_AMOSTRAS;
}

// ── BLE UUIDs ────────────────────────────────────────────────
// Estes UUIDs devem ser idênticos aos definidos no app HTML
#define SERVICE_UUID  "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_UUID     "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer*         pServer         = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool               deviceConnected = false;
bool               oldConnected    = false;

// ── Callbacks de conexão BLE ─────────────────────────────────
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pSrv) override {
    deviceConnected = true;
  }
  void onDisconnect(BLEServer* pSrv) override {
    deviceConnected = false;
  }
};

// ── Variáveis de leitura ─────────────────────────────────────
int   valCalcaneo  = 0;
int   valMeta1     = 0;
int   valMeta5     = 0;
float temperatura  = NAN;
float umidade      = NAN;

unsigned long ultimaLeituraDHT = 0;
const unsigned long INTERVALO_DHT = 2100;  // DHT22 mínimo 2 s entre leituras

// ── setup ────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Wire.begin(5, 4);   // SDA=5, SCL=4 (conforme protótipo)
  dht.begin();

  // — OLED —
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Falha OLED"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.println(F("Iniciando BLE..."));
  display.display();

  // — BLE —
  BLEDevice::init("MonitorPlantar");          // nome visível no app
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHAR_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  // Descritor obrigatório para NOTIFY funcionar no Web Bluetooth
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("BLE ativo. Aguardando conexão...");

  // Aguarda DHT22 estabilizar (recomendação do fabricante)
  delay(2000);
  ultimaLeituraDHT = millis();
}

// ── loop ─────────────────────────────────────────────────────
void loop() {

  // — Leitura FSR (sempre) —
  valCalcaneo = lerSensorMedia(PIN_CALCANEO);
  valMeta1    = lerSensorMedia(PIN_META1);
  valMeta5    = lerSensorMedia(PIN_META5);

  // — Leitura DHT22 (a cada INTERVALO_DHT) —
  if (millis() - ultimaLeituraDHT >= INTERVALO_DHT) {
    float t = dht.readTemperature();
    float u = dht.readHumidity();
    if (!isnan(t)) temperatura = t;
    if (!isnan(u)) umidade     = u;
    ultimaLeituraDHT = millis();
  }

  bool dhtOk = !isnan(temperatura) && !isnan(umidade);

  // — Atualiza OLED —
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println(F("MONITOR PLANTAR"));
  display.drawLine(0, 10, 128, 10, WHITE);

  display.setCursor(0, 13);
  display.printf("Cal: %04d", valCalcaneo);
  display.setCursor(0, 23);
  display.printf("M1:  %04d", valMeta1);
  display.setCursor(0, 33);
  display.printf("M5:  %04d", valMeta5);

  display.drawLine(0, 43, 128, 43, WHITE);

  display.setCursor(0, 46);
  if (dhtOk) {
    display.printf("T:%.1fC  U:%.0f%%", temperatura, umidade);
  } else {
    display.println(F("DHT22: aguardando"));
  }

  // Indicador BLE no canto superior direito
  display.setCursor(104, 0);
  display.println(deviceConnected ? F("BLE:OK") : F("BLE:--"));

  display.display();

  // — Envia JSON via BLE se conectado —
  if (deviceConnected) {
    StaticJsonDocument<128> doc;
    doc["meta1"]    = valMeta1;
    doc["meta5"]    = valMeta5;
    doc["calcaneo"] = valCalcaneo;

    if (dhtOk) {
      // Serializa com 1 casa decimal para economizar bytes
      doc["temp"] = serialized(String(temperatura, 1));
      doc["umid"] = serialized(String(umidade,    1));
    } else {
      doc["temp"] = nullptr;
      doc["umid"] = nullptr;
    }

    char jsonBuffer[128];
    serializeJson(doc, jsonBuffer);

    pCharacteristic->setValue(jsonBuffer);
    pCharacteristic->notify();

    Serial.println(jsonBuffer);   // útil para depuração via Serial Monitor
  }

  // — Reconexão automática após desconexão —
  if (!deviceConnected && oldConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("Reiniciando advertising BLE...");
    oldConnected = false;
  }
  if (deviceConnected && !oldConnected) {
    oldConnected = true;
  }

  // 500 ms entre ciclos — suficiente para DHT22 e para o app
  delay(500);
}
