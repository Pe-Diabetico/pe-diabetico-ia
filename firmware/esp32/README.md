# Firmware ESP32 — Monitor Plantar

Firmware para o ESP32 que lê sensores de pressão plantar (FSR) e
microclima (temperatura e umidade), executa inferência do modelo
Random Forest localmente e transmite os dados via BLE.

---

## Hardware necessário

| Componente | Especificação |
|---|---|
| Microcontrolador | ESP32 (dual-core LX6, 240 MHz, 520 KB RAM) |
| Sensores de pressão | 3x FSR (Force Sensitive Resistor) |
| Sensor de temperatura e umidade | DHT22 |
| Display | OLED SSD1306 128x64 (I2C) |

---

## Pinagem

| Componente | Pino ESP32 | Observação |
|---|---|---|
| FSR Calcâneo | 36 (VP) | Somente entrada analógica |
| FSR Metatarso 1 | 39 (VN) | Somente entrada analógica |
| FSR Metatarso 5 | 33 | |
| DHT22 (data) | 27 | |
| OLED SDA | 5 | |
| OLED SCL | 4 | |

---

## Bibliotecas

Instale pelo Library Manager do Arduino IDE:

- Adafruit SSD1306
- Adafruit GFX Library
- DHT sensor library (Adafruit)
- ArduinoJson 6.x

A biblioteca BLE já está incluída no ESP32.

---

## Arquivo model.h

O modelo Random Forest não está incluído neste repositório.
Ele deve ser gerado antes de compilar o firmware.

Da raiz do projeto:

```bash
python modelo/gerar_dados_sinteticos.py
python modelo/exportar_modelo_esp32.py