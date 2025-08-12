/*
 * Teste Ultra-Simples da Fita LED
 * Para verificar se o hardware está funcionando
 */

#include <Adafruit_NeoPixel.h>

#define PIN_LED      6          // Pino da fita LED
#define NUM_LEDS     20         // Quantidade de LEDs na sua fita

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  Serial.println("Teste LED iniciado");
  
  strip.begin();
  strip.show(); // Limpar LEDs
  
  // Teste básico
  testBasic();
}

void loop() {
  // Ler comandos simples do serial
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch(cmd) {
      case 'r': // Vermelho
        setAllRed();
        break;
      case 'g': // Verde
        setAllGreen();
        break;
      case 'b': // Azul
        setAllBlue();
        break;
      case 'w': // Branco
        setAllWhite();
        break;
      case 'c': // Limpar
        strip.clear();
        strip.show();
        break;
      case '1': // LED 1
        strip.setPixelColor(0, strip.Color(255, 0, 0));
        strip.show();
        break;
      case '2': // LED 2
        strip.setPixelColor(1, strip.Color(0, 255, 0));
        strip.show();
        break;
      case '3': // LED 3
        strip.setPixelColor(2, strip.Color(0, 0, 255));
        strip.show();
        break;
      case 't': // Teste sequencial
        testSequence();
        break;
    }
  }
  
  delay(100);
}

void testBasic() {
  Serial.println("Teste básico iniciado");
  
  // Vermelho
  setAllRed();
  delay(500);
  
  // Verde
  setAllGreen();
  delay(500);
  
  // Azul
  setAllBlue();
  delay(500);
  
  // Branco
  setAllWhite();
  delay(500);
  
  // Limpar
  strip.clear();
  strip.show();
  
  Serial.println("Teste básico concluído");
}

void setAllRed() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }
  strip.show();
  Serial.println("Todos vermelhos");
}

void setAllGreen() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 255, 0));
  }
  strip.show();
  Serial.println("Todos verdes");
}

void setAllBlue() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 255));
  }
  strip.show();
  Serial.println("Todos azuis");
}

void setAllWhite() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 255, 255));
  }
  strip.show();
  Serial.println("Todos brancos");
}

void testSequence() {
  Serial.println("Teste sequencial iniciado");
  
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.clear();
    strip.setPixelColor(i, strip.Color(255, 255, 0)); // Amarelo
    strip.show();
    delay(100);
  }
  
  strip.clear();
  strip.show();
  Serial.println("Teste sequencial concluído");
}
