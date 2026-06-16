/*
 * Teste Final - Sensor de Temperatura e Umidade
 * Biblioteca: Adafruit_AHTX0
 * Correção: Pinos I2C invertidos por software para corrigir a PCB
 */

#include <Wire.h>
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;

void setup() {
  Serial.begin(115200);
  delay(2000); 

  Serial.println("\n[Teste] Iniciando Sensor com Correcao de PCB...");
 
  Wire.begin(21,22);

  // Pequena pausa para a energia estabilizar
  delay(100);

  // Tenta iniciar o sensor
  if (!aht.begin(&Wire)) {
    Serial.println("ERRO: Sensor não respondeu! Verifique se ele está bem encaixado.");
    while (1) delay(10); 
  }

  Serial.println("SUCESSO! O sensor foi detectado.\n");
}

void loop() {
  sensors_event_t umidade, temp;
  
  aht.getEvent(&umidade, &temp);

  Serial.print("Temperatura: ");
  Serial.print(temp.temperature);
  Serial.println(" °C");

  Serial.print("Umidade:     ");
  Serial.print(umidade.relative_humidity);
  Serial.println(" %");

  Serial.println("-------------------------");
  
  delay(2000); 
}