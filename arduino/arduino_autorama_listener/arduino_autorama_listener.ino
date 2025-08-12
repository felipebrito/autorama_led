#include <Adafruit_NeoPixel.h>

// PINAGEM (ajuste se necessário)
#define PIN_LED      6          // pino da fita WS2812/WS2813
#define MAX_PIXELS   300        // capacidade máxima física da fita

// FITA
Adafruit_NeoPixel strip = Adafruit_NeoPixel(MAX_PIXELS, PIN_LED, NEO_GRB + NEO_KHZ800);

// ESTADO E CONFIG
uint16_t numPixels = 20;        // tamanho lógico da pista (recebido via config.maxLed)
uint8_t  tailLen   = 3;         // tamanho do rastro (opcional via config.tail)
uint32_t colorP1   = strip.Color(255, 0, 0);   // carro 1 (vermelho)
uint32_t colorP2   = strip.Color(0, 255, 0);   // carro 2 (verde)
uint32_t colorBG   = strip.Color(0, 0, 0);     // fundo

// Recepção serial (NDJSON: 1 JSON por linha)
static const uint16_t RX_BUF = 256;
char lineBuf[RX_BUF];
uint16_t lineLen = 0;

// Estado vindo do navegador
float dist1 = 0.0f, dist2 = 0.0f;

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
bool hasType(const char* src, const char* typeName) {
  char needle[32];
  snprintf(needle, sizeof(needle), "\"type\":\"%s\"", typeName);
  return strstr(src, needle) != nullptr;
}

void applyConfigFromJson(const char* json) {
  uint16_t v;
  if (parseUInt16(json, "\"maxLed\"", v)) {
    numPixels = clampT<uint16_t>(v, 1, MAX_PIXELS);
  }
  float tail;
  if (parseNumber(json, "\"tail\"", tail)) {
    tailLen = (uint8_t)clampT<int>((int)tail, 0, 20);
  }
}

void applyStateFromJson(const char* json) {
  float v;
  if (parseNumber(json, "\"dist1\"", v)) dist1 = v;
  if (parseNumber(json, "\"dist2\"", v)) dist2 = v;
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

  drawCar((uint16_t)i1, tailLen, colorP1);
  drawCar((uint16_t)i2, tailLen, colorP2);

  strip.show();
}

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show();
  Serial.println("{\"arduino\":\"ready\"}");
}

void loop() {
  // Leitura não bloqueante de linhas
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      lineBuf[clampT<uint16_t>(lineLen, 0, RX_BUF-1)] = '\0';
      if (lineLen > 0) {
        if (hasType(lineBuf, "config")) {
          applyConfigFromJson(lineBuf);
        } else if (hasType(lineBuf, "state")) {
          applyStateFromJson(lineBuf);
          renderFrame();
        }
      }
      lineLen = 0;
    } else if (lineLen < RX_BUF - 1) {
      lineBuf[lineLen++] = c;
    } else {
      // overflow: descarta a linha
      lineLen = 0;
    }
  }
}


