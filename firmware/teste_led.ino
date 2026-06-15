// O LED integrado do Lolin32 geralmente está no pino 2
const int LED_PIN = 2; 

void setup() {
  // Configura o pino do LED como saída
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH); // Liga o LED
  delay(1000);                 // Aguarda 1 segundo (1000 milissegundos)
  
  digitalWrite(LED_PIN, LOW);  // Desliga o LED
  delay(1000);                 // Aguarda mais 1 segundo
}