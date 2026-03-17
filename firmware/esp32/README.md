# Firmware ESP32 para monitoramento plantar

- Placa: ESP32
- Conexões: sensores de pressão, acelerômetro, etc.
- Bibliotecas sugeridas: `BLEDevice`, `BLEServer`, `BLE2902`, `Adafruit_MPR121` (exemplo).

## Pinagem

- GPIO 32: entrada sensor 1
- GPIO 33: entrada sensor 2
- GPIO 34: entrada sensor 3
- GPIO 35: entrada sensor 4

## Uso

1. Faça upload do sketch para o ESP32.
2. Configure BLE e envie leituras ao app Web.
