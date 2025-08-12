/*
 * Arduino Autorama Listener - Versão Corrigida e Simplificada
 * Baseada no led_test_simple.ino que funciona
 */

#include <Adafruit_NeoPixel.h>

// PINAGEM (ajuste se necessário)
#define PIN_LED      6          // pino da fita WS2812/WS2813
#define MAX_PIXELS   300        // capacidade máxima suportada
#define PHYS_PIXELS  20         // QUANTIDADE REAL de LEDs na sua fita

// FITA
Adafruit_NeoPixel strip = Adafruit_NeoPixel(MAX_PIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);

// ESTADO E CONFIG
uint16_t numPixels = PHYS_PIXELS;
uint8_t  tailLen   = 3;

// Estado vindo do navegador
float dist1 = 0.0f, dist2 = 0.0f;
unsigned long lastMsgMs = 0;
bool haveLiveData = false;
bool gameRunning = false;

// Buffer para receber mensagens
static const uint16_t RX_BUF = 512;
char lineBuf[RX_BUF];
uint16_t lineLen = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Arduino Autorama Listener iniciando...");
  
  strip.begin();
  strip.show(); // Limpar LEDs
  
  // Teste visual de inicialização
  testLEDs();
  
  Serial.println("{\"arduino\":\"ready\",\"leds\":20}");
  Serial.println("{\"status\":\"Arduino iniciado e aguardando comandos\"}");
}

void loop() {
  // Leitura não bloqueante de linhas
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      lineBuf[lineLen] = '\0';
      if (lineLen > 0) {
        processMessage(lineBuf);
        lastMsgMs = millis();
        haveLiveData = true;
      }
      lineLen = 0;
    } else if (lineLen < RX_BUF - 1) {
      lineBuf[lineLen++] = c;
    } else {
      lineLen = 0; // overflow: descarta a linha
    }
  }

  // Demo: só roda se não tiver dados vivos
  if (!haveLiveData || (millis() - lastMsgMs > 3000)) {
    if (haveLiveData && gameRunning) {
      // Se temos dados vivos E jogo ativo, NÃO roda demo
      return;
    }
    runDemo();
  }
}

void processMessage(const char* message) {
  Serial.print("{\"debug\":\"RX:\"");
  Serial.print(message);
  Serial.println("\"}");
  
  if (strstr(message, "\"type\":\"config\"")) {
    handleConfig(message);
  } else if (strstr(message, "\"type\":\"state\"")) {
    handleState(message);
    renderFrame();
  } else if (strstr(message, "\"type\":\"effect\"")) {
    handleEffect();
  } else if (strstr(message, "\"type\":\"ping\"")) {
    handlePing();
  } else {
    Serial.println("{\"debug\":\"Tipo nao reconhecido\"}");
  }
}

void handleConfig(const char* message) {
  Serial.println("{\"debug\":\"Processando config\"}");
  
  // Extrair maxLed
  int maxLed = extractValue(message, "maxLed");
  if (maxLed > 0) {
    numPixels = (maxLed < PHYS_PIXELS) ? maxLed : PHYS_PIXELS;
    Serial.print("{\"debug\":\"maxLed setado para:\"");
    Serial.print(numPixels);
    Serial.println("\"}");
  }
  
  // Extrair tail
  int tail = extractValue(message, "tail");
  if (tail > 0) {
    tailLen = (tail < 20) ? tail : 20;
    Serial.print("{\"debug\":\"tail setado para:\"");
    Serial.print(tailLen);
    Serial.println("\"}");
  }
  
  // Confirmar config
  showConfigReceived();
}

void handleState(const char* message) {
  Serial.println("{\"debug\":\"Processando state\"}");
  
  // Extrair dist1
  float d1 = extractFloat(message, "dist1");
  if (d1 >= 0) {
    dist1 = d1;
    Serial.print("{\"debug\":\"dist1:\"");
    Serial.print(dist1);
    Serial.println("\"}");
  }
  
  // Extrair dist2
  float d2 = extractFloat(message, "dist2");
  if (d2 >= 0) {
    dist2 = d2;
    Serial.print("{\"debug\":\"dist2:\"");
    Serial.print(dist2);
    Serial.println("\"}");
  }
  
  // Extrair running
  if (strstr(message, "\"running\"")) {
    float r = extractFloat(message, "running");
    gameRunning = (r != 0.0f);
    Serial.print("{\"debug\":\"running setado para:\"");
    Serial.print(gameRunning ? "true" : "false");
    Serial.println("\"}");
  }
}

void handleEffect() {
  Serial.println("{\"debug\":\"Processando effect\"}");
  // Flash branco em todos os LEDs
  for (uint16_t i = 0; i < numPixels; i++) {
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
  int center = numPixels / 2;
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
  int i1 = ((long)floor(dist1) % numPixels + numPixels) % numPixels;
  int i2 = ((long)floor(dist2) % numPixels + numPixels) % numPixels;
  
  Serial.print("{\"debug\":\"Posicoes - i1:\"");
  Serial.print(i1);
  Serial.print(", i2:\"");
  Serial.print(i2);
  Serial.print(", numPixels:\"");
  Serial.print(numPixels);
  Serial.println("\"}");
  
  // Desenhar carro 1 (vermelho)
  strip.setPixelColor(i1, strip.Color(255, 0, 0));
  // Rastro do carro 1
  for (uint8_t i = 1; i <= tailLen; i++) {
    uint16_t p = (i1 + i) % numPixels;
    strip.setPixelColor(p, strip.Color(100, 0, 0));
  }
  
  // Desenhar carro 2 (verde)
  strip.setPixelColor(i2, strip.Color(0, 255, 0));
  // Rastro do carro 2
  for (uint8_t i = 1; i <= tailLen; i++) {
    uint16_t p = (i2 + i) % numPixels;
    strip.setPixelColor(p, strip.Color(0, 100, 0));
  }
  
  Serial.println("{\"debug\":\"Carros desenhados, chamando strip.show()\"}");
  strip.show();
  Serial.println("{\"debug\":\"strip.show() executado\"}");
}

// Funções auxiliares para extrair valores
int extractValue(const char* message, const char* key) {
  const char* k = strstr(message, key);
  if (!k) return -1;
  k = strchr(k, ':'); if (!k) return -1; k++;
  while (*k==' '||*k=='\t') k++;
  char* endptr = nullptr;
  return strtol(k, &endptr, 10);
}

float extractFloat(const char* message, const char* key) {
  const char* k = strstr(message, key);
  if (!k) return -1.0f;
  k = strchr(k, ':'); if (!k) return -1.0f; k++;
  while (*k==' '||*k=='\t') k++;
  char* endptr = nullptr;
  return strtof(k, &endptr);
}

void showConfigReceived() {
  // Piscar verde para confirmar config
  for (uint16_t i = 0; i < numPixels; i++) {
    strip.setPixelColor(i, strip.Color(0, 100, 0));
  }
  strip.show();
  delay(200);
  strip.clear();
  strip.show();
}

void testLEDs() {
  // Sequência RGB para testar todos os LEDs
  for (int i = 0; i < numPixels; i++) {
    strip.setPixelColor(i, strip.Color(50, 0, 0));   // Vermelho
  }
  strip.show();
  delay(300);
  
  for (int i = 0; i < numPixels; i++) {
    strip.setPixelColor(i, strip.Color(0, 50, 0));   // Verde
  }
  strip.show();
  delay(300);
  
  for (int i = 0; i < numPixels; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 50));   // Azul
  }
  strip.show();
  delay(300);
  
  // Limpar
  strip.clear();
  strip.show();
}

void runDemo() {
  static unsigned long lastStep = 0;
  static uint16_t demoIdx = 0;
  
  if (millis() - lastStep > 100) {
    strip.clear();
    strip.setPixelColor(demoIdx % numPixels, strip.Color(30, 30, 0));
    strip.show();
    demoIdx++;
    lastStep = millis();
  }
}


