/*
 * Arduino Autorama - Versão Ultra-Simples com Testes Visuais
 * Baseado no led_test_simple.ino que FUNCIONA
 * Padrões visuais únicos para cada comando - não precisa de Serial Monitor
 */

#include <Adafruit_NeoPixel.h>

#define PIN_LED      6          // Pino da fita LED
#define NUM_LEDS     20         // Quantidade de LEDs na sua fita

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

// Estado do jogo
float dist1 = 0.0f, dist2 = 0.0f;
bool gameRunning = false;
bool gameJustStarted = false;
unsigned long gameStartTime = 0;

// Buffer para receber mensagens
String inputString = "";
bool stringComplete = false;

// Contador para testes visuais
unsigned long lastTestTime = 0;
int testCounter = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  strip.begin();
  strip.show(); // Limpar LEDs
  
  // Teste visual de inicialização - SEQUÊNCIA RGB
  testLEDs();
  
  // Sinal de "pronto" - PISCAR VERDE 3x
  for (int flash = 0; flash < 3; flash++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color(0, 255, 0));
    }
    strip.show();
    delay(200);
    strip.clear();
    strip.show();
    delay(200);
  }
}

void loop() {
  // Ler mensagens do serial
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
  
  // Processar mensagem completa
  if (stringComplete) {
    processMessage(inputString);
    inputString = "";
    stringComplete = false;
  }
  
  // Countdown visual quando o jogo iniciar
  if (gameJustStarted && (millis() - gameStartTime < 3000)) {
    runCountdown();
  } else if (gameJustStarted) {
    gameJustStarted = false;
  }
  
  // Teste visual periódico para confirmar que está funcionando
  if (millis() - lastTestTime > 5000) {
    runPeriodicTest();
    lastTestTime = millis();
  }
  
  // Demo simples se não receber mensagens
  static unsigned long lastDemo = 0;
  if (millis() - lastDemo > 2000) {
    runDemo();
    lastDemo = millis();
  }
}

void processMessage(String message) {
  // Verificar tipo de mensagem
  if (message.indexOf("\"type\":\"config\"") >= 0) {
    handleConfig(message);
  } else if (message.indexOf("\"type\":\"state\"") >= 0) {
    handleState(message);
    renderFrame();
  } else if (message.indexOf("\"type\":\"effect\"") >= 0) {
    handleEffect();
  } else if (message.indexOf("\"type\":\"ping\"") >= 0) {
    handlePing();
  }
}

void handleConfig(String message) {
  // PADRÃO VISUAL: PISCAR VERDE 2x (config recebida)
  for (int flash = 0; flash < 2; flash++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color(0, 255, 0));
    }
    strip.show();
    delay(150);
    strip.clear();
    strip.show();
    delay(150);
  }
}

void handleState(String message) {
  // Extrair dist1
  int start = message.indexOf("\"dist1\":");
  if (start >= 0) {
    start = message.indexOf(":", start) + 1;
    while (start < message.length() && (message.charAt(start) == ' ' || message.charAt(start) == '\t')) {
      start++;
    }
    int end = start;
    while (end < message.length() && (isDigit(message.charAt(end)) || message.charAt(end) == '-' || message.charAt(end) == '.')) {
      end++;
    }
    dist1 = message.substring(start, end).toFloat();
  }
  
  // Extrair dist2
  start = message.indexOf("\"dist2\":");
  if (start >= 0) {
    start = message.indexOf(":", start) + 1;
    while (start < message.length() && (message.charAt(start) == ' ' || message.charAt(start) == '\t')) {
      start++;
    }
    int end = start;
    while (end < message.length() && (isDigit(message.charAt(end)) || message.charAt(end) == '-' || message.charAt(end) == '.')) {
      end++;
    }
    dist2 = message.substring(start, end).toFloat();
  }
  
  // Extrair running
  if (message.indexOf("\"running\":") >= 0) {
    start = message.indexOf("\"running\":");
    start = message.indexOf(":", start) + 1;
    while (start < message.length() && (message.charAt(start) == ' ' || message.charAt(start) == '\t')) {
      start++;
    }
    int end = start;
    while (end < message.length() && (isDigit(message.charAt(end)) || message.charAt(end) == '-' || message.charAt(end) == '.')) {
      end++;
    }
    float r = message.substring(start, end).toFloat();
    bool wasRunning = gameRunning;
    gameRunning = (r != 0.0f);
    
    // Se o jogo acabou de iniciar
    if (gameRunning && !wasRunning) {
      gameJustStarted = true;
      gameStartTime = millis();
      // PADRÃO VISUAL: PISCAR BRANCO 3x (jogo iniciado)
      for (int flash = 0; flash < 3; flash++) {
        for (int i = 0; i < NUM_LEDS; i++) {
          strip.setPixelColor(i, strip.Color(255, 255, 255));
        }
        strip.show();
        delay(100);
        strip.clear();
        strip.show();
        delay(100);
      }
    }
  }
}

void handleEffect() {
  // PADRÃO VISUAL: FLASH BRANCO RÁPIDO (effect recebido)
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 255, 255));
  }
  strip.show();
  delay(80);
  strip.clear();
  strip.show();
  delay(80);
  strip.show();
  delay(80);
  strip.clear();
  strip.show();
}

void handlePing() {
  // PADRÃO VISUAL: PISCAR AZUL NO CENTRO (ping recebido)
  int center = NUM_LEDS / 2;
  strip.setPixelColor(center, strip.Color(0, 0, 255));
  strip.show();
  delay(200);
  strip.setPixelColor(center, strip.Color(0, 0, 0));
  strip.show();
}

void renderFrame() {
  strip.clear();
  
  // Calcular posições
  int i1 = ((long)floor(dist1) % NUM_LEDS + NUM_LEDS) % NUM_LEDS;
  int i2 = ((long)floor(dist2) % NUM_LEDS + NUM_LEDS) % NUM_LEDS;
  
  // Desenhar carro 1 (vermelho)
  strip.setPixelColor(i1, strip.Color(255, 0, 0));
  
  // Desenhar carro 2 (verde)
  strip.setPixelColor(i2, strip.Color(0, 255, 0));
  
  strip.show();
}

void runCountdown() {
  static int countdownStep = 0;
  static unsigned long lastCountdown = 0;
  
  if (millis() - lastCountdown > 1000) {
    lastCountdown = millis();
    
    // Limpar LEDs
    strip.clear();
    
    // Piscar toda a fita com cores diferentes
    uint32_t color;
    switch(countdownStep) {
      case 0: // Vermelho
        color = strip.Color(255, 0, 0);
        break;
      case 1: // Amarelo
        color = strip.Color(255, 255, 0);
        break;
      case 2: // Verde
        color = strip.Color(0, 255, 0);
        break;
    }
    
    // Acender toda a fita
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, color);
    }
    strip.show();
    
    countdownStep++;
    if (countdownStep >= 3) {
      countdownStep = 0;
    }
  }
}

void runPeriodicTest() {
  // Teste visual periódico - PISCAR AZUL ALTERNADO
  strip.clear();
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i % 2 == testCounter % 2) {
      strip.setPixelColor(i, strip.Color(0, 0, 100));
    }
  }
  strip.show();
  delay(100);
  strip.clear();
  strip.show();
  
  testCounter++;
}

void testLEDs() {
  // SEQUÊNCIA RGB para testar todos os LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(50, 0, 0));   // Vermelho
  }
  strip.show();
  delay(300);
  
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 50, 0));   // Verde
  }
  strip.show();
  delay(300);
  
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 50));   // Azul
  }
  strip.show();
  delay(300);
  
  // Limpar
  strip.clear();
  strip.show();
}

void runDemo() {
  static int demoPos = 0;
  strip.clear();
  strip.setPixelColor(demoPos % NUM_LEDS, strip.Color(30, 30, 0));
  strip.show();
  demoPos++;
}
