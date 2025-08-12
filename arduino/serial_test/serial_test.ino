/*
 * Teste Simples de Comunicação Serial
 * Para testar conexão com o navegador via Web Serial API
 */

// Configurações básicas
#define PIN_LED      6          // Pino da fita LED
#define NUM_LEDS     20         // Quantidade de LEDs na sua fita

// Incluir biblioteca NeoPixel
#include <Adafruit_NeoPixel.h>

// Criar objeto da fita LED
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

// Variáveis de estado
bool connected = false;
unsigned long lastMessageTime = 0;
unsigned long heartbeatTime = 0;

// Buffer para receber mensagens
String inputString = "";
bool stringComplete = false;

void setup() {
  // Inicializar comunicação serial
  Serial.begin(115200);
  
  // Aguardar um pouco para estabilizar
  delay(1000);
  
  // Inicializar fita LED
  strip.begin();
  strip.show(); // Limpar LEDs
  
  // Teste visual de inicialização
  testLEDs();
  
  // Enviar mensagem de "pronto"
  Serial.println("{\"arduino\":\"ready\",\"leds\":" + String(NUM_LEDS) + "}");
  
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
    lastMessageTime = millis();
    connected = true;
  }
  
  // Heartbeat a cada 5 segundos se conectado
  if (connected && (millis() - heartbeatTime > 5000)) {
    Serial.println("{\"heartbeat\":\"" + String(millis()) + "\"}");
    heartbeatTime = millis();
  }
  
  // Demo simples se não receber mensagens por 10 segundos
  if (millis() - lastMessageTime > 10000) {
    runDemo();
  }
  
  // Pequeno delay para estabilidade
  delay(10);
}

void processMessage(String message) {
  Serial.println("{\"received\":\"" + message + "\"}");
  
  // Verificar tipo de mensagem
  if (message.indexOf("\"type\":\"config\"") >= 0) {
    handleConfig(message);
  } else if (message.indexOf("\"type\":\"state\"") >= 0) {
    handleState(message);
  } else if (message.indexOf("\"type\":\"ping\"") >= 0) {
    handlePing(message);
  } else {
    Serial.println("{\"error\":\"Tipo de mensagem não reconhecido\"}");
  }
}

void handleConfig(String message) {
  Serial.println("{\"status\":\"Configuração recebida\"}");
  
  // Extrair valores da configuração
  int maxLed = extractValue(message, "maxLed");
  int tail = extractValue(message, "tail");
  
  if (maxLed > 0) {
    Serial.println("{\"config\":{\"maxLed\":" + String(maxLed) + ",\"tail\":" + String(tail) + "}}");
  }
  
  // Mostrar visualmente que config foi recebida
  showConfigReceived();
}

void handleState(String message) {
  Serial.println("{\"status\":\"Estado recebido\"}");
  
  // Extrair valores do estado
  float dist1 = extractFloat(message, "dist1");
  float dist2 = extractFloat(message, "dist2");
  int running = extractValue(message, "running");
  
  Serial.println("{\"state\":{\"dist1\":" + String(dist1) + ",\"dist2\":" + String(dist2) + ",\"running\":" + String(running) + "}}");
  
  // Mostrar posições dos carros
  showCarPositions(dist1, dist2, running);
}

void handlePing(String message) {
  Serial.println("{\"pong\":\"" + String(millis()) + "\"}");
  
  // Piscar LED para confirmar ping
  blinkLED();
}

// Funções auxiliares para extrair valores
int extractValue(String message, String key) {
  int start = message.indexOf("\"" + key + "\":");
  if (start == -1) return -1;
  
  start = message.indexOf(":", start) + 1;
  while (start < message.length() && (message.charAt(start) == ' ' || message.charAt(start) == '\t')) {
    start++;
  }
  
  int end = start;
  while (end < message.length() && (isDigit(message.charAt(end)) || message.charAt(end) == '-')) {
    end++;
  }
  
  return message.substring(start, end).toInt();
}

float extractFloat(String message, String key) {
  int start = message.indexOf("\"" + key + "\":");
  if (start == -1) return -1.0;
  
  start = message.indexOf(":", start) + 1;
  while (start < message.length() && (message.charAt(start) == ' ' || message.charAt(start) == '\t')) {
    start++;
  }
  
  int end = start;
  while (end < message.length() && (isDigit(message.charAt(end)) || message.charAt(end) == '-' || message.charAt(end) == '.')) {
    end++;
  }
  
  return message.substring(start, end).toFloat();
}

// Funções visuais
void testLEDs() {
  // Sequência RGB para testar todos os LEDs
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

void showConfigReceived() {
  // Piscar verde para confirmar config
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 100, 0));
  }
  strip.show();
  delay(200);
  strip.clear();
  strip.show();
}

void showCarPositions(float dist1, float dist2, int running) {
  strip.clear();
  
  if (running) {
    // Calcular posições dos carros
    int pos1 = (int)dist1 % NUM_LEDS;
    int pos2 = (int)dist2 % NUM_LEDS;
    
    // Garantir que as posições estejam dentro do range
    if (pos1 < 0) pos1 += NUM_LEDS;
    if (pos2 < 0) pos2 += NUM_LEDS;
    
    // Desenhar carro 1 (vermelho)
    strip.setPixelColor(pos1, strip.Color(255, 0, 0));
    
    // Desenhar carro 2 (verde)
    strip.setPixelColor(pos2, strip.Color(0, 255, 0));
    
    // Rastro do carro 1
    for (int i = 1; i <= 2; i++) {
      int trailPos = (pos1 - i + NUM_LEDS) % NUM_LEDS;
      strip.setPixelColor(trailPos, strip.Color(100, 0, 0));
    }
    
    // Rastro do carro 2
    for (int i = 1; i <= 2; i++) {
      int trailPos = (pos2 - i + NUM_LEDS) % NUM_LEDS;
      strip.setPixelColor(trailPos, strip.Color(0, 100, 0));
    }
  }
  
  strip.show();
}

void runDemo() {
  static int demoPos = 0;
  static unsigned long lastDemo = 0;
  
  if (millis() - lastDemo > 100) {
    strip.clear();
    strip.setPixelColor(demoPos, strip.Color(255, 255, 0)); // Amarelo
    strip.show();
    
    demoPos = (demoPos + 1) % NUM_LEDS;
    lastDemo = millis();
  }
}

void blinkLED() {
  // Piscar LED central para confirmar ping
  int center = NUM_LEDS / 2;
  strip.setPixelColor(center, strip.Color(255, 255, 255)); // Branco
  strip.show();
  delay(100);
  strip.setPixelColor(center, strip.Color(0, 0, 0));
  strip.show();
}
