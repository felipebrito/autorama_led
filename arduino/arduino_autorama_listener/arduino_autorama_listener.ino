/*
 * Arduino Autorama Listener - Corrigido
 * Para receber dados do navegador via Web Serial API
 */

// Incluir biblioteca NeoPixel
#include <Adafruit_NeoPixel.h>

// PINAGEM (ajuste se necessário)
#define PIN_LED      6          // pino da fita WS2812/WS2813
#define MAX_PIXELS   300        // capacidade máxima suportada pelo objeto NeoPixel
// AJUSTE AQUI: quantidade REAL de LEDs na sua fita conectada
#ifndef PHYS_PIXELS
#define PHYS_PIXELS  20         // EX.: 20 LEDs físicos conectados
#endif

// FITA
Adafruit_NeoPixel strip = Adafruit_NeoPixel(MAX_PIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);

// ESTADO E CONFIG
uint16_t numPixels = PHYS_PIXELS; // tamanho lógico da pista (recebido via config.maxLed e limitado aos LEDs físicos)
uint8_t  tailLen   = 3;         // tamanho do rastro (opcional via config.tail)
uint32_t colorP1   = strip.Color(255, 0, 0);   // carro 1 (vermelho)
uint32_t colorP2   = strip.Color(0, 255, 0);   // carro 2 (verde)
uint32_t colorBG   = strip.Color(0, 0, 0);     // fundo

// Recepção serial (NDJSON: 1 JSON por linha)
static const uint16_t RX_BUF = 512;  // Aumentado para mensagens maiores
char lineBuf[RX_BUF];
uint16_t lineLen = 0;

// Estado vindo do navegador
float dist1 = 0.0f, dist2 = 0.0f;
unsigned long lastMsgMs = 0;   // última msg válida recebida
bool haveLiveData = false;     // já recebemos 'state'
bool gameRunning = false;      // vindo do navegador

template<typename T>
T clampT(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); }

// Parse simples de número em JSON: procura "key":<número>
bool parseNumber(const char* src, const char* key, float& out) {
  const char* k = strstr(src, key);
  if (!k) return false;
  k = strchr(k, ':'); if (!k) return false; k++;
  while (*k==' '||*k=='\t') k++;
  char* endptr = nullptr;
  out = strtod(k, &endptr);
  return endptr != k;
}

bool parseUInt16(const char* src, const char* key, uint16_t& out) {
  float tmp = 0;
  if (!parseNumber(src, key, tmp)) return false;
  out = (uint16_t)(tmp + 0.5f);
  return true;
}

// Função mais robusta para detectar tipo de mensagem
bool hasType(const char* src, const char* typeName) {
  char needle[64];
  snprintf(needle, sizeof(needle), "\"type\":\"%s\"", typeName);
  return strstr(src, needle) != nullptr;
}

void applyConfigFromJson(const char* json) {
  uint16_t v;
  if (parseUInt16(json, "\"maxLed\"", v)) {
    // Nunca ultrapassar a quantidade FÍSICA de LEDs conectados
    uint16_t limit = (PHYS_PIXELS < MAX_PIXELS) ? PHYS_PIXELS : MAX_PIXELS;
    numPixels = clampT<uint16_t>(v, 1, limit);
    Serial.print("{\"debug\":\"maxLed setado para:\"");
    Serial.print(numPixels);
    Serial.println("\"}");
  }
  float tail;
  if (parseNumber(json, "\"tail\"", tail)) {
    tailLen = (uint8_t)clampT<int>((int)tail, 0, 20);
    Serial.print("{\"debug\":\"tail setado para:\"");
    Serial.print(tailLen);
    Serial.println("\"}");
  }
}

void applyStateFromJson(const char* json) {
  float v;
  if (parseNumber(json, "\"dist1\"", v)) {
    dist1 = v;
    Serial.print("{\"debug\":\"dist1:\"");
    Serial.print(dist1);
    Serial.println("\"}");
  }
  if (parseNumber(json, "\"dist2\"", v)) {
    dist2 = v;
    Serial.print("{\"debug\":\"dist2:\"");
    Serial.print(dist2);
    Serial.println("\"}");
  }
  // running opcional
  if (strstr(json, "\"running\"")) {
    float r = 0;
    if (parseNumber(json, "\"running\"", r)) {
      gameRunning = (r != 0.0f);
      Serial.print("{\"debug\":\"running setado para:\"");
      Serial.print(gameRunning ? "true" : "false");
      Serial.println("\"}");
    } else {
      // se vier como booleano true/false textual, faz fallback simples
      if (strstr(json, "\"running\":true")) {
        gameRunning = true;
        Serial.println("{\"debug\":\"running fallback para true\"}");
      }
      if (strstr(json, "\"running\":false")) {
        gameRunning = false;
        Serial.println("{\"debug\":\"running fallback para false\"}");
      }
    }
  }
}

void drawCar(uint16_t idx, uint8_t tail, uint32_t color) {
  for (uint8_t i = 0; i <= tail; i++) {
    uint16_t p = (idx + i) % numPixels;
    strip.setPixelColor(p, color);
  }
}

void renderFrame() {
  strip.clear();
  // índices atuais (alinhados à pista lógica)
  int i1 = ((long)floor(dist1) % numPixels + numPixels) % numPixels;
  int i2 = ((long)floor(dist2) % numPixels + numPixels) % numPixels;

  // DEBUG: Log das posições calculadas
  Serial.print("{\"debug\":\"renderFrame - pos1:\"");
  Serial.print(i1);
  Serial.print(", pos2:\"");
  Serial.print(i2);
  Serial.print(", numPixels:\"");
  Serial.print(numPixels);
  Serial.println("\"}");

  drawCar((uint16_t)i1, tailLen, colorP1);
  drawCar((uint16_t)i2, tailLen, colorP2);

  strip.show();
}

void showColor(uint8_t r, uint8_t g, uint8_t b, uint16_t count, uint16_t delayMs) {
  uint32_t c = strip.Color(r,g,b);
  count = (count > MAX_PIXELS) ? MAX_PIXELS : count;
  for (uint16_t i = 0; i < count; i++) strip.setPixelColor(i, c);
  strip.show();
  delay(delayMs);
}

void selfTest() {
  // Sequência curta para validar alimentação/dados
  showColor(50, 0, 0, numPixels, 300);
  showColor(0, 50, 0, numPixels, 300);
  showColor(0, 0, 50, numPixels, 300);
  // limpa
  strip.clear();
  strip.show();
}

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show();
  selfTest();
  Serial.println("{\"arduino\":\"ready\",\"leds\":20}");
  Serial.println("{\"status\":\"Arduino iniciado e aguardando comandos\"}");
}

void loop() {
  // Leitura não bloqueante de linhas
  bool gotLine = false;
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      lineBuf[clampT<uint16_t>(lineLen, 0, RX_BUF-1)] = '\0';
      if (lineLen > 0) {
        // DEBUG: Log de todas as mensagens recebidas
        Serial.print("{\"debug\":\"RX:\"");
        Serial.print(lineBuf);
        Serial.println("\"}");
        
        if (hasType(lineBuf, "config")) {
          Serial.println("{\"debug\":\"Processando config\"}");
          applyConfigFromJson(lineBuf);
        } else if (hasType(lineBuf, "state")) {
          Serial.println("{\"debug\":\"Processando state\"}");
          applyStateFromJson(lineBuf);
          renderFrame();
          haveLiveData = true;
          lastMsgMs = millis();
        } else {
          Serial.println("{\"debug\":\"Tipo nao reconhecido\"}");
        }
      }
      lineLen = 0;
      gotLine = true;
    } else if (lineLen < RX_BUF - 1) {
      lineBuf[lineLen++] = c;
    } else {
      // overflow: descarta a linha
      lineLen = 0;
    }
  }

  // Demo: se não receber dados por 2s OU se running=false, roda demo
  if (!haveLiveData || (millis() - lastMsgMs > 2000) || !gameRunning) {
    static unsigned long lastStep = 0;
    static uint16_t demoIdx = 0;
    if (millis() - lastStep > 80) {
      lastStep = millis();
      strip.clear();
      strip.setPixelColor(demoIdx % numPixels, strip.Color(30, 30, 0));
      strip.show();
      demoIdx++;
    }
  }
}


