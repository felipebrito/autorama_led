/*
 * Arduino Autorama - Versão Ultra-Simples
 * Baseado no led_test_simple.ino que FUNCIONA
 */

#include <Adafruit_NeoPixel.h>

#define PIN_LED      6          // Pino da fita LED
#define NUM_LEDS     20         // Quantidade de LEDs na sua fita

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

// Estado do jogo
float dist1 = 0.0f, dist2 = 0.0f;
bool gameRunning = false;

// Buffer para receber mensagens
String inputString = "";
bool stringComplete = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Arduino Autorama iniciando...");
  
  strip.begin();
  strip.show(); // Limpar LEDs
  
  // Teste visual de inicialização
  testLEDs();
  
  Serial.println("{\"arduino\":\"ready\",\"leds\":20}");
  Serial.println("{\"status\":\"Arduino iniciado e aguardando comandos\"}");
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
  
  // Demo simples se não receber mensagens
  static unsigned long lastDemo = 0;
  if (millis() - lastDemo > 2000) {
    runDemo();
    lastDemo = millis();
  }
}

void processMessage(String message) {
  Serial.print("{\"debug\":\"RX:\"");
  Serial.print(message);
  Serial.println("\"}");
  
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
  } else {
    Serial.println("{\"debug\":\"Tipo nao reconhecido\"}");
  }
}

void handleConfig(String message) {
  Serial.println("{\"debug\":\"Processando config\"}");
  // Piscar verde para confirmar
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 100, 0));
  }
  strip.show();
  delay(200);
  strip.clear();
  strip.show();
}

void handleState(String message) {
  Serial.println("{\"debug\":\"Processando state\"}");
  
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
    Serial.print("{\"debug\":\"dist1:\"");
    Serial.print(dist1);
    Serial.println("\"}");
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
    Serial.print("{\"debug\":\"dist2:\"");
    Serial.print(dist2);
    Serial.println("\"}");
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
    gameRunning = (r != 0.0f);
    Serial.print("{\"debug\":\"running setado para:\"");
    Serial.print(gameRunning ? "true" : "false");
    Serial.println("\"}");
  }
}

void handleEffect() {
  Serial.println("{\"debug\":\"Processando effect\"}");
  // Flash branco em todos os LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(80, 80, 80));
  }
  strip.show();
  delay(120);
  strip.clear();
  strip.show();
}

void handlePing() {
  Serial.println("{\"pong\":\"ok\"}");
  // Piscar LED central
  int center = NUM_LEDS / 2;
  strip.setPixelColor(center, strip.Color(255, 255, 255));
  strip.show();
  delay(80);
  strip.setPixelColor(center, strip.Color(0, 0, 0));
  strip.show();
}

void renderFrame() {
  Serial.println("{\"debug\":\"renderFrame iniciado\"}");
  
  strip.clear();
  
  // Calcular posições
  int i1 = ((long)floor(dist1) % NUM_LEDS + NUM_LEDS) % NUM_LEDS;
  int i2 = ((long)floor(dist2) % NUM_LEDS + NUM_LEDS) % NUM_LEDS;
  
  Serial.print("{\"debug\":\"Posicoes - i1:\"");
  Serial.print(i1);
  Serial.print(", i2:\"");
  Serial.print(i2);
  Serial.print(", numPixels:\"");
  Serial.print(NUM_LEDS);
  Serial.println("\"}");
  
  // Desenhar carro 1 (vermelho)
  strip.setPixelColor(i1, strip.Color(255, 0, 0));
  
  // Desenhar carro 2 (verde)
  strip.setPixelColor(i2, strip.Color(0, 255, 0));
  
  Serial.println("{\"debug\":\"Carros desenhados, chamando strip.show()\"}");
  strip.show();
  Serial.println("{\"debug\":\"strip.show() executado\"}");
}

void testLEDs() {
  Serial.println("Teste LED iniciado");
  
  // Vermelho
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(50, 0, 0));
  }
  strip.show();
  delay(300);
  
  // Verde
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 50, 0));
  }
  strip.show();
  delay(300);
  
  // Azul
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 50));
  }
  strip.show();
  delay(300);
  
  // Limpar
  strip.clear();
  strip.show();
  
  Serial.println("Teste LED concluído");
}

void runDemo() {
  static int demoPos = 0;
  strip.clear();
  strip.setPixelColor(demoPos % NUM_LEDS, strip.Color(30, 30, 0));
  strip.show();
  demoPos++;
}
