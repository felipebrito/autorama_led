/*
 * Teste de LEDs - Teste Básico da Fita LED
 * Verifica se todos os LEDs estão funcionando
 * Comandos via Serial Monitor: r, g, b, c, t
 */

#include <Adafruit_NeoPixel.h>

#define PIN_LED      6          // Pino da fita LED
#define NUM_LEDS     20         // Quantidade de LEDs na sua fita

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== TESTE DE LEDs ===");
  Serial.println("Comandos disponíveis:");
  Serial.println("r - Teste vermelho");
  Serial.println("g - Teste verde");
  Serial.println("b - Teste azul");
  Serial.println("c - Limpar LEDs");
  Serial.println("t - Teste completo");
  Serial.println("===================");
  
  strip.begin();
  strip.show(); // Limpar LEDs
  
  // Teste inicial
  testeInicial();
}

void loop() {
  if (Serial.available()) {
    char comando = Serial.read();
    
    switch (comando) {
      case 'r':
      case 'R':
        testeVermelho();
        break;
        
      case 'g':
      case 'G':
        testeVerde();
        break;
        
      case 'b':
      case 'B':
        testeAzul();
        break;
        
      case 'c':
      case 'C':
        limparLEDs();
        break;
        
      case 't':
      case 'T':
        testeCompleto();
        break;
        
      default:
        if (comando != '\n' && comando != '\r') {
          Serial.print("Comando não reconhecido: ");
          Serial.println(comando);
        }
        break;
    }
  }
}

void testeInicial() {
  Serial.println("Executando teste inicial...");
  
  // Sequência RGB
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(50, 0, 0));   // Vermelho
  }
  strip.show();
  delay(500);
  
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 50, 0));   // Verde
  }
  strip.show();
  delay(500);
  
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 50));   // Azul
  }
  strip.show();
  delay(500);
  
  // Limpar
  strip.clear();
  strip.show();
  
  Serial.println("Teste inicial concluído!");
}

void testeVermelho() {
  Serial.println("Teste VERMELHO...");
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }
  strip.show();
  delay(1000);
  strip.clear();
  strip.show();
  Serial.println("Teste vermelho concluído!");
}

void testeVerde() {
  Serial.println("Teste VERDE...");
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 255, 0));
  }
  strip.show();
  delay(1000);
  strip.clear();
  strip.show();
  Serial.println("Teste verde concluído!");
}

void testeAzul() {
  Serial.println("Teste AZUL...");
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 255));
  }
  strip.show();
  delay(1000);
  strip.clear();
  strip.show();
  Serial.println("Teste azul concluído!");
}

void limparLEDs() {
  Serial.println("Limpando LEDs...");
  strip.clear();
  strip.show();
  Serial.println("LEDs limpos!");
}

void testeCompleto() {
  Serial.println("Executando teste COMPLETO...");
  
  // Teste individual de cada LED
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.clear();
    strip.setPixelColor(i, strip.Color(255, 255, 255));
    strip.show();
    delay(100);
  }
  
  // Teste de cores
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }
  strip.show();
  delay(500);
  
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 255, 0));
  }
  strip.show();
  delay(500);
  
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 255));
  }
  strip.show();
  delay(500);
  
  strip.clear();
  strip.show();
  Serial.println("Teste completo concluído!");
}
