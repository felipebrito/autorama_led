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

// CONFIGURAÇÕES
#define PIN_LED      2          // GPIO 2 para fita LED
#define PIN_BTN1     4          // GPIO 4 para botão Jogador 1
#define PIN_BTN2     5          // GPIO 5 para botão Jogador 2
#define NUM_LEDS     20         // Quantidade de LEDs
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
int maxLed = 100;
bool gameRunning = false;
bool testMode = false;
bool flag_sw1 = true, flag_sw2 = true; // Edge detection como no original
bool btn1LastState = false, btn2LastState = false;
unsigned long lastBtn1Press = 0, lastBtn2Press = 0;

// CONFIGURAÇÕES FÍSICAS
float acel = 0.2f;      // Aceleração
float kf = 0.015f;      // Atrito
float kg = 0.003f;      // Gravidade
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
  strip.begin();
  strip.show();
  Serial.println("✓ LEDs inicializados");
  
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
      Serial.printf("Jogo: P1(%.1f, %.1f, %d) P2(%.1f, %.1f, %d)\n", 
                   dist1, vel1, loop1, dist2, vel2, loop2);
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
  } else if (message.equalsIgnoreCase("ajuda")) {
    Serial.println("✓ Comando AJUDA detectado");
    showHelp();
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
    delay(100);
  }
  
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
  Serial.println("ajuda     - Mostra esta ajuda");
  Serial.println("========================\n");
}

// ===== FUNÇÕES DO JOGO =====
void readButtons() {
  // Edge detection como no código original
  if ((flag_sw1 == 1) && (digitalRead(PIN_BTN1) == 0)) {
    flag_sw1 = 0;
    vel1 += acel;
    Serial.println("Botão Jogador 1 pressionado!");
  }
  if ((flag_sw1 == 0) && (digitalRead(PIN_BTN1) == 1)) {
    flag_sw1 = 1;
  }

  if ((flag_sw2 == 1) && (digitalRead(PIN_BTN2) == 0)) {
    flag_sw2 = 0;
    vel2 += acel;
    Serial.println("Botão Jogador 2 pressionado!");
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
  html += "};";
  html += "</script>";
  html += "</body></html>";
  return html;
}
