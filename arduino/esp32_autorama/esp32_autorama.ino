/*
 * ESP32 Autorama - Controle de LEDs via WiFi + Serial
 * ESP32 WROOM com comunicação dual
 * WiFi: Web Server integrado
 * Serial: Backup e debug
 */

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

// Verificar se a biblioteca está funcionando
#define NEO_PIXEL_TEST

// CONFIGURAÇÕES
#define PIN_LED      2          // GPIO 2 para fita LED
#define PIN_BTN1     4          // GPIO 4 para botão Jogador 1
#define PIN_BTN2     5          // GPIO 5 para botão Jogador 2
#define NUM_LEDS     50         // Quantidade de LEDs (aumentado para voltas mais longas)
#define WIFI_SSID    "Autorama_LED"  // Nome da rede WiFi
#define WIFI_PASS    "12345678"      // Senha da rede WiFi
#define WEB_PORT     80
#define WS_PORT      81

// OBJETOS
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);
WebServer server(WEB_PORT);
WebSocketsServer webSocket = WebSocketsServer(WS_PORT);

// ESTADO DO JOGO
float dist1 = 0.0f, dist2 = 0.0f;
float vel1 = 0.0f, vel2 = 0.0f;
int loop1 = 0, loop2 = 0;
int loopMax = 5;
int maxLed = 50;  // Consistente com NUM_LEDS
bool gameRunning = false;
bool testMode = false;
bool flag_sw1 = true, flag_sw2 = true; // Edge detection como no original
bool btn1LastState = false, btn2LastState = false;
unsigned long lastBtn1Press = 0, lastBtn2Press = 0;

// DEBOUNCE
unsigned long lastBtn1Debounce = 0, lastBtn2Debounce = 0;
unsigned long DEBOUNCE_DELAY = 150; // 150ms de debounce (ajustável)

// CONFIGURAÇÕES FÍSICAS
float acel = 0.05f;     // Aceleração (reduzida de 0.2 para 0.05)
float kf = 0.02f;       // Atrito (aumentado para desacelerar mais)
float kg = 0.005f;      // Gravidade (aumentada para desacelerar mais)
int tail = 3;           // Rastro visual

// TIMING
unsigned long lapStart1 = 0, lapStart2 = 0;
unsigned long bestLap1 = 0, bestLap2 = 0;
unsigned long lastLap1 = 0, lastLap2 = 0;

// BUFFER SERIAL
String inputString = "";
bool stringComplete = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== ESP32 AUTORAMA ===");
  Serial.println("Inicializando...");
  
  // INICIALIZAR LEDS
  Serial.printf("Inicializando LEDs: PIN=%d, NUM_LEDS=%d\n", PIN_LED, NUM_LEDS);
  strip.begin();
  strip.show();
  Serial.printf("✓ LEDs inicializados - Total: %d pixels\n", strip.numPixels());
  
  // Teste rápido de inicialização
  Serial.println("Teste rápido de inicialização...");
  for (int i = 0; i < 5; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }
  strip.show();
  delay(500);
  strip.clear();
  strip.show();
  Serial.println("✓ Teste de inicialização concluído");
  
  // CONFIGURAR BOTÕES
  pinMode(PIN_BTN1, INPUT_PULLUP);
  pinMode(PIN_BTN2, INPUT_PULLUP);
  Serial.println("✓ Botões configurados");
  
  // TESTE VISUAL INICIAL
  testLEDs();
  
  // CONFIGURAR WI-FI
  setupWiFi();
  
  // CONFIGURAR WEB SERVER
  setupWebServer();
  
  // CONFIGURAR WEB SOCKET
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  Serial.println("✓ ESP32 inicializado com sucesso!");
  Serial.println("✓ WiFi: " + WiFi.localIP().toString());
  Serial.println("✓ Web Server: http://" + WiFi.localIP().toString());
  Serial.println("✓ Serial: 115200 baud");
  
  showHelp();
}

void loop() {
  // PROCESSAR WEB SERVER
  server.handleClient();
  
  // PROCESSAR WEB SOCKET
  webSocket.loop();
  
  // PROCESSAR SERIAL
  processSerial();
  
  // MODOS DE TESTE
  if (testMode) {
    runTestMode();
  }
  
  // DEMO PERIÓDICO (DESATIVADO - SÓ MANUAL)
  // static unsigned long lastDemo = 0;
  // if (millis() - lastDemo > 10000 && !testMode) {
  //   runDemo();
  //   lastDemo = millis();
  // }
  
  // MANTER WI-FI ATIVO (AP mode não precisa reconectar)
  static unsigned long lastWiFiCheck = 0;
  if (millis() - lastWiFiCheck > 30000) { // Verificar a cada 30 segundos
    lastWiFiCheck = millis();
    if (WiFi.softAPgetStationNum() == 0) {
      Serial.println("Nenhum dispositivo conectado ao WiFi");
    } else {
      Serial.printf("Dispositivos conectados: %d\n", WiFi.softAPgetStationNum());
    }
  }
  
  // LER BOTÕES FÍSICOS
  readButtons();
  
  // ATUALIZAR JOGO
  if (gameRunning) {
    updateGame();
    
    // Debug a cada segundo
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 1000) {
      lastDebug = millis();
      Serial.printf("Jogo: P1(%.1f, %.1f, %d) P2(%.1f, %.1f, %d) | Debounce: %lu ms\n", 
                   dist1, vel1, loop1, dist2, vel2, loop2, DEBOUNCE_DELAY);
    }
  }
}

// ===== WI-FI =====
void setupWiFi() {
  Serial.print("Configurando WiFi AP: ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("WiFi AP configurado com sucesso!");
}

// ===== WEB SERVER =====
void setupWebServer() {
  // PÁGINA PRINCIPAL
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", getMainPage());
  });
  
  // API PARA COMANDOS
  server.on("/api/command", HTTP_POST, handleCommand);
  
  // INICIAR SERVER
  server.begin();
  Serial.println("✓ Web Server iniciado");
}

// ===== WEB SOCKET =====
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Desconectado!\n", num);
      break;
    case WStype_CONNECTED:
      Serial.printf("[%u] Conectado!\n", num);
      break;
    case WStype_TEXT:
      handleWebSocketMessage(num, payload, length);
      break;
  }
}

// ===== SERIAL =====
void processSerial() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
  
  if (stringComplete) {
    processMessage(inputString);
    inputString = "";
    stringComplete = false;
  }
}

// ===== PROCESSAMENTO DE MENSAGENS =====
void processMessage(String message) {
  message.trim();
  Serial.print("Processando mensagem: '");
  Serial.print(message);
  Serial.println("'");
  
  // COMANDOS SIMPLES
  if (message.equalsIgnoreCase("demo")) {
    Serial.println("✓ Comando DEMO detectado");
    runDemo();
  } else if (message.equalsIgnoreCase("jogador1")) {
    Serial.println("✓ Comando JOGADOR1 detectado");
    testJogador1();
  } else if (message.equalsIgnoreCase("jogador2")) {
    Serial.println("✓ Comando JOGADOR2 detectado");
    testJogador2();
  } else if (message.equalsIgnoreCase("start")) {
    Serial.println("✓ Comando START detectado");
    startGame();
  } else if (message.equalsIgnoreCase("stop")) {
    Serial.println("✓ Comando STOP detectado");
    stopGame();
  } else if (message.equalsIgnoreCase("limpar")) {
    Serial.println("✓ Comando LIMPAR detectado");
    strip.clear();
    strip.show();
    Serial.println("✓ LEDs limpos");
  } else if (message.equalsIgnoreCase("teste")) {
    Serial.println("✓ Comando TESTE detectado");
    // Teste simples: acender todos os LEDs vermelhos
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color(255, 0, 0));
    }
    strip.show();
    Serial.printf("✓ Todos os %d LEDs acendidos em vermelho\n", NUM_LEDS);
    delay(2000);
    strip.clear();
    strip.show();
  } else if (message.equalsIgnoreCase("btn1")) {
    Serial.println("✓ Botão Jogador 1 via browser");
    vel1 += acel;
    Serial.printf("Jogador 1 acelerou: vel=%.2f\n", vel1);
    
    // Feedback visual imediato
    strip.clear();
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color(255, 0, 0)); // Vermelho
    }
    strip.show();
    delay(200);
    strip.clear();
    strip.show();
    
  } else if (message.equalsIgnoreCase("btn2")) {
    Serial.println("✓ Botão Jogador 2 via browser");
    vel2 += acel;
    Serial.printf("Jogador 2 acelerou: vel=%.2f\n", vel2);
    
    // Feedback visual imediato
    strip.clear();
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color(0, 255, 0)); // Verde
    }
    strip.show();
    delay(200);
    strip.clear();
    strip.show();
  } else if (message.equalsIgnoreCase("ajuda")) {
    Serial.println("✓ Comando AJUDA detectado");
    showHelp();
  } else if (message.startsWith("debounce")) {
    // Comando: debounce 200 (ajusta debounce para 200ms)
    int newDelay = message.substring(9).toInt();
    if (newDelay > 0 && newDelay < 1000) {
      DEBOUNCE_DELAY = newDelay;
      Serial.printf("✓ Debounce ajustado para %d ms\n", DEBOUNCE_DELAY);
    } else {
      Serial.printf("Debounce atual: %lu ms\n", DEBOUNCE_DELAY);
    }
  } else if (message.startsWith("acel")) {
    // Comando: acel 0.1 (ajusta aceleração)
    float newAcel = message.substring(5).toFloat();
    if (newAcel > 0.01f && newAcel < 1.0f) {
      acel = newAcel;
      Serial.printf("✓ Aceleração ajustada para %.3f\n", acel);
    } else {
      Serial.printf("Aceleração atual: %.3f\n", acel);
    }
  } else if (message.startsWith("{") && message.endsWith("}")) {
    // COMANDOS JSON
    processJSON(message);
  } else {
    Serial.print("✗ Comando não reconhecido: '");
    Serial.print(message);
    Serial.println("'");
  }
}

// ===== PROCESSAMENTO JSON =====
void processJSON(String json) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    Serial.println("✗ Erro ao parsear JSON");
    return;
  }
  
  String type = doc["type"];
  
  if (type == "config") {
    Serial.println("✓ Comando CONFIG detectado");
    handleConfig(doc);
  } else if (type == "state") {
    Serial.println("✓ Comando STATE detectado");
    handleState(doc);
  } else if (type == "effect") {
    Serial.println("✓ Comando EFFECT detectado");
    handleEffect(doc);
  } else if (type == "ping") {
    Serial.println("✓ Comando PING detectado");
    handlePing(doc);
  }
}

// ===== HANDLERS =====
void handleConfig(DynamicJsonDocument &doc) {
  Serial.println("Configuracao recebida:");
  Serial.print("  maxLed: "); Serial.println(doc["maxLed"].as<int>());
  Serial.print("  loopMax: "); Serial.println(doc["loopMax"].as<int>());
  
  // Confirmação visual
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 255, 0));
  }
  strip.show();
  delay(500);
  strip.clear();
  strip.show();
}

void handleState(DynamicJsonDocument &doc) {
  dist1 = doc["dist1"];
  dist2 = doc["dist2"];
  gameRunning = doc["running"];
  
  Serial.printf("Estado: d1=%.2f, d2=%.2f, running=%d\n", dist1, dist2, gameRunning);
  
  // Renderizar LEDs baseado no estado
  renderGameState();
}

void handleEffect(DynamicJsonDocument &doc) {
  String effectName = doc["name"];
  Serial.print("Efeito: "); Serial.println(effectName);
  
  if (effectName == "flash") {
    // Flash branco
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color(255, 255, 255));
    }
    strip.show();
    delay(200);
    strip.clear();
    strip.show();
  }
}

void handlePing(DynamicJsonDocument &doc) {
  Serial.println("Ping recebido!");
  
  // Resposta visual
  strip.setPixelColor(0, strip.Color(0, 0, 255));
  strip.show();
  delay(100);
  strip.clear();
  strip.show();
}

// ===== FUNÇÕES DE TESTE =====
void testLEDs() {
  Serial.println("Testando LEDs...");
  
  // Teste individual
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.clear();
    strip.setPixelColor(i, strip.Color(255, 255, 255));
    strip.show();
    delay(50);
  }
  
  // Teste de cores
  uint32_t colors[] = {strip.Color(255,0,0), strip.Color(0,255,0), strip.Color(0,0,255)};
  for (int c = 0; c < 3; c++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, colors[c]);
    }
    strip.show();
    delay(300);
  }
  
  strip.clear();
  strip.show();
  Serial.println("✓ Teste de LEDs concluído");
}

void testJogador1() {
  Serial.println("Testando Jogador 1 (vermelho)...");
  Serial.printf("NUM_LEDS: %d, PIN_LED: %d\n", NUM_LEDS, PIN_LED);
  
  // Teste simples primeiro
  Serial.println("Teste 1: LED único vermelho");
  strip.clear();
  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.show();
  delay(1000);
  
  // Teste movimento
  Serial.println("Teste 2: Movimento com rastro");
  for (int pos = 0; pos < NUM_LEDS; pos++) {
    strip.clear();
    strip.setPixelColor(pos, strip.Color(255, 0, 0));
    
    // Rastro
    for (int i = 1; i <= 3; i++) {
      int rastro = pos - i;
      if (rastro >= 0) {
        strip.setPixelColor(rastro, strip.Color(50, 0, 0));
      }
    }
    
    strip.show();
    Serial.printf("Pos: %d, LEDs ativos: %d\n", pos, strip.numPixels());
    delay(200);
  }
  
  // Manter último estado por 2 segundos
  Serial.println("Teste 3: Estado final");
  strip.clear();
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }
  strip.show();
  delay(2000);
  
  strip.clear();
  strip.show();
  Serial.println("✓ Teste Jogador 1 concluído");
}

void testJogador2() {
  Serial.println("Testando Jogador 2 (verde)...");
  
  for (int pos = 0; pos < NUM_LEDS; pos++) {
    strip.clear();
    strip.setPixelColor(pos, strip.Color(0, 255, 0));
    
    // Rastro
    for (int i = 1; i <= 3; i++) {
      int rastro = pos - i;
      if (rastro >= 0) {
        strip.setPixelColor(rastro, strip.Color(0, 50, 0));
      }
    }
    
    strip.show();
    delay(100);
  }
  
  strip.clear();
  strip.show();
  Serial.println("✓ Teste Jogador 2 concluído");
}

void startGame() {
  Serial.println("Iniciando jogo...");
  gameRunning = true;
  dist1 = 0.0f;
  dist2 = 0.0f;
  vel1 = 0.0f;
  vel2 = 0.0f;
  loop1 = 0;
  loop2 = 0;
  flag_sw1 = true;
  flag_sw2 = true;
  
  // Countdown visual
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      strip.setPixelColor(j, strip.Color(255, 255, 0)); // Amarelo
    }
    strip.show();
    delay(500);
    strip.clear();
    strip.show();
    delay(500);
  }
  
  Serial.println("✓ Jogo iniciado! Primeiro a 5 voltas vence!");
}

void stopGame() {
  Serial.println("Parando jogo...");
  gameRunning = false;
  
  // Efeito de parada
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0)); // Vermelho
  }
  strip.show();
  delay(1000);
  strip.clear();
  strip.show();
  
  Serial.println("✓ Jogo parado!");
}

void runDemo() {
  Serial.println("Executando demo...");
  
  uint32_t colors[] = {strip.Color(255,0,0), strip.Color(0,255,0), strip.Color(0,0,255)};
  for (int c = 0; c < 3; c++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, colors[c]);
    }
    strip.show();
    delay(200);
  }
  
  strip.clear();
  strip.show();
}

void runTestMode() {
  // Indicador visual de teste
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 500) {
    strip.setPixelColor(0, strip.Color(255, 255, 0));
    strip.show();
    lastBlink = millis();
  }
}

void renderGameState() {
  if (!gameRunning) return;
  
  strip.clear();
  
  // Renderizar posição dos carros com draworder alternado (como no original)
  static bool draworder = false;
  if ((millis() & 512) == (512 * draworder)) {
    if (draworder == 0) {
      draworder = 1;
    } else {
      draworder = 0;
    }
  }
  
  // Renderizar na ordem correta
  if (draworder == 0) {
    drawCar1();
    drawCar2();
  } else {
    drawCar2();
    drawCar1();
  }
  
  strip.show();
}

void drawCar1() {
  for (int i = 0; i <= tail; i++) {
    int pos = ((int)dist1 % NUM_LEDS) + i;
    if (pos < NUM_LEDS) {
      int intensidade = 255 - (i * 20);
      if (intensidade > 0) {
        strip.setPixelColor(pos, strip.Color(255 - i * 20, 0, 0)); // Vermelho com fade
      }
    }
  }
}

void drawCar2() {
  for (int i = 0; i <= tail; i++) {
    int pos = ((int)dist2 % NUM_LEDS) + i;
    if (pos < NUM_LEDS) {
      int intensidade = 255 - (i * 20);
      if (intensidade > 0) {
        strip.setPixelColor(pos, strip.Color(0, 255 - i * 20, 0)); // Verde com fade
      }
    }
  }
}

void showHelp() {
  Serial.println("\n=== COMANDOS DISPONIVEIS ===");
  Serial.println("demo      - Animacao demo");
  Serial.println("jogador1  - Testa carro vermelho");
  Serial.println("jogador2  - Testa carro verde");
  Serial.println("start     - Inicia o jogo");
  Serial.println("stop      - Para o jogo");
  Serial.println("limpar    - Limpa todos os LEDs");
  Serial.println("teste     - Teste simples dos LEDs");
  Serial.println("btn1      - Acelerar Jogador 1 (browser)");
  Serial.println("btn2      - Acelerar Jogador 2 (browser)");
  Serial.println("debounce 200 - Ajusta debounce (ms)");
  Serial.println("acel 0.1  - Ajusta aceleração");
  Serial.println("ajuda     - Mostra esta ajuda");
  Serial.println("========================\n");
}

// ===== FUNÇÕES DO JOGO =====
void readButtons() {
  // DEBOUNCE Jogador 1
  if ((flag_sw1 == 1) && (digitalRead(PIN_BTN1) == 0)) {
    if (millis() - lastBtn1Debounce > DEBOUNCE_DELAY) {
      flag_sw1 = 0;
      vel1 += acel;
      lastBtn1Debounce = millis();
      Serial.println("Botão Jogador 1 pressionado!");
    }
  }
  if ((flag_sw1 == 0) && (digitalRead(PIN_BTN1) == 1)) {
    flag_sw1 = 1;
  }

  // DEBOUNCE Jogador 2
  if ((flag_sw2 == 1) && (digitalRead(PIN_BTN2) == 0)) {
    if (millis() - lastBtn2Debounce > DEBOUNCE_DELAY) {
      flag_sw2 = 0;
      vel2 += acel;
      lastBtn2Debounce = millis();
      Serial.println("Botão Jogador 2 pressionado!");
    }
  }
  if ((flag_sw2 == 0) && (digitalRead(PIN_BTN2) == 1)) {
    flag_sw2 = 1;
  }
}

void updateGame() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate < 50) return; // 20 FPS
  lastUpdate = millis();
  
  // Botões já processados em readButtons()
  
  // Aplicar física (velocidade e distância)
  dist1 += vel1;
  dist2 += vel2;
  
  // Aplicar atrito e gravidade
  vel1 *= (1.0f - kf);
  vel2 *= (1.0f - kf);
  
  // Aplicar gravidade (desaceleração natural)
  if (vel1 > 0) vel1 -= kg;
  if (vel2 > 0) vel2 -= kg;
  
  // Garantir velocidade não negativa
  if (vel1 < 0) vel1 = 0;
  if (vel2 < 0) vel2 = 0;
  
  // Verificar volta completa (como no original)
  if (dist1 > NUM_LEDS * loop1) {
    loop1++;
    tone(PIN_LED, 600); // Som de volta (usando LED como speaker temporário)
    Serial.printf("Jogador 1 completou volta %d!\n", loop1);
  }
  
  if (dist2 > NUM_LEDS * loop2) {
    loop2++;
    tone(PIN_LED, 700); // Som de volta (usando LED como speaker temporário)
    Serial.printf("Jogador 2 completou volta %d!\n", loop2);
  }
  
  // Verificar vitória (5 voltas)
  if (loop1 >= loopMax || loop2 >= loopMax) {
    gameRunning = false;
    if (loop1 >= loopMax) {
      Serial.println("Jogador 1 venceu a corrida!");
      victoryEffect(1);
    } else {
      Serial.println("Jogador 2 venceu a corrida!");
      victoryEffect(2);
    }
    return;
  }
  
  // Renderizar estado atual
  renderGameState();
  
  // Enviar estado via WebSocket com todas as informações
  String stateMsg = "{\"type\":\"state\",\"dist1\":" + String(dist1) + ",\"dist2\":" + String(dist2) + 
                   ",\"vel1\":" + String(vel1) + ",\"vel2\":" + String(vel2) + 
                   ",\"loop1\":" + String(loop1) + ",\"loop2\":" + String(loop2) + 
                   ",\"running\":1}";
  webSocket.broadcastTXT(stateMsg);
}

void victoryEffect(int player) {
  uint32_t color = (player == 1) ? strip.Color(255, 0, 0) : strip.Color(0, 255, 0);
  
  // Piscar LEDs da cor do vencedor
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      strip.setPixelColor(j, color);
    }
    strip.show();
    delay(200);
    strip.clear();
    strip.show();
    delay(200);
  }
}

// ===== WEB SERVER HANDLERS =====
void handleCommand() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    Serial.print("Comando recebido via web: ");
    Serial.println(body);
    
    processMessage(body);
    
    server.send(200, "application/json", "{\"status\":\"ok\"}");
    Serial.println("✓ Resposta enviada para web");
  } else {
    Serial.println("✗ Erro: Sem corpo na requisição");
    server.send(400, "application/json", "{\"error\":\"No body\"}");
  }
}

// ===== WEB SOCKET HANDLERS =====
void handleWebSocketMessage(uint8_t num, uint8_t * payload, size_t length) {
  String message = String((char*)payload);
  Serial.printf("WebSocket [%u]: ", num);
  Serial.println(message);
  
  processMessage(message);
}

// ===== PÁGINA HTML =====
String getMainPage() {
  String html = "<!DOCTYPE html>";
  html += "<html><head>";
  html += "<title>ESP32 Autorama</title>";
  html += "<meta charset=\"UTF-8\">";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }";
  html += ".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }";
  html += ".status { padding: 10px; margin: 10px 0; border-radius: 5px; }";
  html += ".connected { background: #d4edda; color: #155724; }";
  html += ".disconnected { background: #f8d7da; color: #721c24; }";
  html += "button { background: #007bff; color: white; border: none; padding: 10px 20px; margin: 5px; border-radius: 5px; cursor: pointer; }";
  html += "button:hover { background: #0056b3; }";
  html += ".led-track { display: flex; margin: 20px 0; overflow-x: auto; }";
  html += ".led { width: 20px; height: 20px; border: 1px solid #ccc; margin: 2px; border-radius: 50%; background: #333; }";
  html += ".log { background: #f8f9fa; border: 1px solid #dee2e6; padding: 15px; height: 200px; overflow-y: auto; font-family: monospace; }";
  html += "</style></head><body>";
  html += "<div class=\"container\">";
  html += "<h1>ESP32 Autorama</h1>";
  html += "<div class=\"status disconnected\" id=\"status\">Desconectado</div>";
  html += "<div>";
  html += "<button onclick=\"connect()\">Conectar</button>";
  html += "<button onclick=\"disconnect()\">Desconectar</button>";
  html += "</div>";
  html += "<h3>Controles</h3>";
  html += "<div>";
  html += "<button onclick=\"sendCommand('demo')\">Demo</button>";
  html += "<button onclick=\"sendCommand('jogador1')\">Jogador 1</button>";
  html += "<button onclick=\"sendCommand('jogador2')\">Jogador 2</button>";
  html += "<button onclick=\"sendCommand('start')\">Start</button>";
  html += "<button onclick=\"sendCommand('stop')\">Stop</button>";
  html += "<button onclick=\"sendCommand('limpar')\">Limpar</button>";
  html += "<button onclick=\"sendCommand('teste')\">Teste LEDs</button>";
  html += "</div>";
  
  html += "<h3>Controle dos Jogadores (Browser)</h3>";
  html += "<div style=\"margin: 20px 0;\">";
  html += "<button onclick=\"sendCommand('btn1')\" style=\"background: #ff4444; font-size: 18px; padding: 15px 30px; margin: 10px;\">🚗 ACELERAR JOGADOR 1</button>";
  html += "<button onclick=\"sendCommand('btn2')\" style=\"background: #44ff44; font-size: 18px; padding: 15px 30px; margin: 10px;\">🏎️ ACELERAR JOGADOR 2</button>";
  html += "</div>";
  
  html += "<div style=\"background: #f8f9fa; padding: 15px; border-radius: 5px; margin: 20px 0;\">";
  html += "<strong>💡 Dica:</strong> Use estes botões para testar se o problema é nos botões físicos ou no código dos LEDs.";
  html += "</div>";
  
  html += "<h3>Ajustes em Tempo Real</h3>";
  html += "<div style=\"margin: 20px 0;\">";
  html += "<button onclick=\"sendCommand('acel 0.02')\" style=\"background: #ff8800; margin: 5px;\">Aceleração Baixa (0.02)</button>";
  html += "<button onclick=\"sendCommand('acel 0.05')\" style=\"background: #ffaa00; margin: 5px;\">Aceleração Média (0.05)</button>";
  html += "<button onclick=\"sendCommand('acel 0.1')\" style=\"background: #ffcc00; margin: 5px;\">Aceleração Alta (0.1)</button>";
  html += "<button onclick=\"sendCommand('debounce 100')\" style=\"background: #00aaff; margin: 5px;\">Debounce Rápido (100ms)</button>";
  html += "<button onclick=\"sendCommand('debounce 200')\" style=\"background: #0088ff; margin: 5px;\">Debounce Lento (200ms)</button>";
  html += "</div>";
  html += "<h3>Estado do Jogo</h3>";
  html += "<div>";
  html += "<label>Jogador 1: <span id=\"dist1\">0.00</span></label><br>";
  html += "<label>Jogador 2: <span id=\"dist2\">0.00</span></label><br>";
  html += "<label>Status: <span id=\"gameStatus\">Parado</span></label>";
  html += "</div>";
  html += "<h3>Pista LED</h3>";
  html += "<div class=\"led-track\" id=\"ledTrack\"></div>";
  html += "<h3>Log</h3>";
  html += "<div class=\"log\" id=\"log\"></div>";
  html += "</div>";
  html += "<script>";
  html += "let ws = null;";
  html += "let ledElements = [];";
  html += "function initLEDs() {";
  html += "  const track = document.getElementById('ledTrack');";
  html += "  track.innerHTML = '';";
  html += "  ledElements = [];";
  html += "  for (let i = 0; i < 20; i++) {";
  html += "    const led = document.createElement('div');";
  html += "    led.className = 'led';";
  html += "    led.id = 'led-' + i;";
  html += "    track.appendChild(led);";
  html += "    ledElements.push(led);";
  html += "  }";
  html += "}";
  html += "function connect() {";
  html += "  const ip = window.location.hostname;";
  html += "  ws = new WebSocket('ws://' + ip + ':81');";
  html += "  ws.onopen = function() {";
  html += "    updateStatus('Conectado', true);";
  html += "    log('WebSocket conectado');";
  html += "  };";
  html += "  ws.onmessage = function(event) {";
  html += "    const data = JSON.parse(event.data);";
  html += "    log('Recebido: ' + JSON.stringify(data));";
  html += "    if (data.type === 'state') {";
  html += "      updateGameState(data);";
  html += "    }";
  html += "  };";
  html += "  ws.onclose = function() {";
  html += "    updateStatus('Desconectado', false);";
  html += "    log('WebSocket desconectado');";
  html += "  };";
  html += "  ws.onerror = function(error) {";
  html += "    log('Erro: ' + error);";
  html += "  };";
  html += "}";
  html += "function disconnect() {";
  html += "  if (ws) {";
  html += "    ws.close();";
  html += "    ws = null;";
  html += "  }";
  html += "}";
  html += "function sendCommand(cmd) {";
  html += "  if (ws && ws.readyState === WebSocket.OPEN) {";
  html += "    ws.send(cmd);";
  html += "    log('Enviado: ' + cmd);";
  html += "  } else {";
  html += "    log('Erro: Nao conectado');";
  html += "  }";
  html += "}";
  html += "function updateStatus(text, connected) {";
  html += "  const status = document.getElementById('status');";
  html += "  status.textContent = text;";
  html += "  status.className = 'status ' + (connected ? 'connected' : 'disconnected');";
  html += "}";
  html += "function updateGameState(data) {";
  html += "  document.getElementById('dist1').textContent = data.dist1.toFixed(2);";
  html += "  document.getElementById('dist2').textContent = data.dist2.toFixed(2);";
  html += "  document.getElementById('gameStatus').textContent = data.running ? 'Rodando' : 'Parado';";
  html += "  updateLEDs(data);";
  html += "}";
  html += "function updateLEDs(data) {";
  html += "  ledElements.forEach(led => led.style.background = '#333');";
  html += "  const pos1 = Math.floor((data.dist1 / 100) * 19);";
  html += "  const pos2 = Math.floor((data.dist2 / 100) * 19);";
  html += "  if (pos1 >= 0 && pos1 < 20) {";
  html += "    ledElements[pos1].style.background = '#ff0000';";
  html += "  }";
  html += "  if (pos2 >= 0 && pos2 < 20) {";
  html += "    ledElements[pos2].style.background = '#00ff00';";
  html += "  }";
  html += "}";
  html += "function log(message) {";
  html += "  const logDiv = document.getElementById('log');";
  html += "  const timestamp = new Date().toLocaleTimeString();";
  html += "  logDiv.innerHTML += '[' + timestamp + '] ' + message + '<br>';";
  html += "  logDiv.scrollTop = logDiv.scrollHeight;";
  html += "}";
  html += "window.onload = function() {";
  html += "  initLEDs();";
  html += "  log('Pagina carregada');";
  html += "  log('💡 Use Q para Jogador 1, P para Jogador 2');";
  html += "};";
  
  html += "// Teclas de atalho";
  html += "document.addEventListener('keydown', function(event) {";
  html += "  if (event.key.toLowerCase() === 'q') {";
  html += "    event.preventDefault();";
  html += "    sendCommand('btn1');";
  html += "    log('Tecla Q pressionada - Jogador 1');";
  html += "  }";
  html += "  if (event.key.toLowerCase() === 'p') {";
  html += "    event.preventDefault();";
  html += "    sendCommand('btn2');";
  html += "    log('Tecla P pressionada - Jogador 2');";
  html += "  }";
  html += "});";
  html += "</script>";
  html += "</body></html>";
  return html;
}
