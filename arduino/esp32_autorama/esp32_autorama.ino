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
bool gameRunning = false;
bool testMode = false;

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
  
  // DEMO PERIÓDICO
  static unsigned long lastDemo = 0;
  if (millis() - lastDemo > 10000 && !testMode) {
    runDemo();
    lastDemo = millis();
  }
  
  // MANTER WI-FI ATIVO
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi perdido, reconectando...");
    setupWiFi();
  }
}

// ===== WI-FI =====
void setupWiFi() {
  Serial.print("Conectando ao WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.softAPIP());
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
  } else if (message.equalsIgnoreCase("simular")) {
    Serial.println("✓ Comando SIMULAR detectado");
    startSimulation();
  } else if (message.equalsIgnoreCase("limpar")) {
    Serial.println("✓ Comando LIMPAR detectado");
    strip.clear();
    strip.show();
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
  Serial.println("Configuração recebida:");
  Serial.print("  maxLed: "); Serial.println(doc["maxLed"]);
  Serial.print("  loopMax: "); Serial.println(doc["loopMax"]);
  
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

void startSimulation() {
  Serial.println("Iniciando simulação...");
  testMode = true;
  
  // Simular dados do jogo
  for (int step = 0; step < 20; step++) {
    float d1 = step * 1.5;
    float d2 = step * 1.2;
    
    // Enviar via WebSocket
    String stateMsg = "{\"type\":\"state\",\"dist1\":" + String(d1) + ",\"dist2\":" + String(d2) + ",\"running\":1}";
    webSocket.broadcastTXT(stateMsg);
    
    Serial.printf("Step %d: d1=%.1f, d2=%.1f\n", step, d1, d2);
    delay(300);
  }
  
  testMode = false;
  Serial.println("✓ Simulação concluída");
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
  
  // Renderizar posição dos carros
  int pos1 = map(dist1, 0, 100, 0, NUM_LEDS-1);
  int pos2 = map(dist2, 0, 100, 0, NUM_LEDS-1);
  
  strip.setPixelColor(pos1, strip.Color(255, 0, 0));  // Jogador 1 vermelho
  strip.setPixelColor(pos2, strip.Color(0, 255, 0));  // Jogador 2 verde
  
  strip.show();
}

void showHelp() {
  Serial.println("\n=== COMANDOS DISPONÍVEIS ===");
  Serial.println("demo      - Animação demo");
  Serial.println("jogador1  - Testa carro vermelho");
  Serial.println("jogador2  - Testa carro verde");
  Serial.println("simular   - Simula dados do jogo");
  Serial.println("limpar    - Limpa todos os LEDs");
  Serial.println("ajuda     - Mostra esta ajuda");
  Serial.println("========================\n");
}

// ===== WEB SERVER HANDLERS =====
void handleCommand() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    Serial.print("Comando recebido via web: ");
    Serial.println(body);
    
    processMessage(body);
    
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "application/json", "{\"error\":\"No body\"}");
  }
}

// ===== WEB SOCKET HANDLERS =====
void handleWebSocketMessage(uint8_t num, uint8_t * payload, size_t length) {
  String message = String((char*)payload);
  Serial.print("WebSocket: ");
  Serial.println(message);
  
  processMessage(message);
}

// ===== PÁGINA HTML =====
String getMainPage() {
  return R"(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Autorama</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        .status { padding: 10px; margin: 10px 0; border-radius: 5px; }
        .connected { background: #d4edda; color: #155724; }
        .disconnected { background: #f8d7da; color: #721c24; }
        button { background: #007bff; color: white; border: none; padding: 10px 20px; margin: 5px; border-radius: 5px; cursor: pointer; }
        button:hover { background: #0056b3; }
        .led-track { display: flex; margin: 20px 0; overflow-x: auto; }
        .led { width: 20px; height: 20px; border: 1px solid #ccc; margin: 2px; border-radius: 50%; background: #333; }
        .log { background: #f8f9fa; border: 1px solid #dee2e6; padding: 15px; height: 200px; overflow-y: auto; font-family: monospace; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🚗 ESP32 Autorama</h1>
        
        <div class="status disconnected" id="status">Desconectado</div>
        
        <div>
            <button onclick="connect()">🔌 Conectar</button>
            <button onclick="disconnect()">❌ Desconectar</button>
        </div>
        
        <h3>🎮 Controles</h3>
        <div>
            <button onclick="sendCommand('demo')">🎬 Demo</button>
            <button onclick="sendCommand('jogador1')">🔴 Jogador 1</button>
            <button onclick="sendCommand('jogador2')">🟢 Jogador 2</button>
            <button onclick="sendCommand('simular')">🎯 Simular</button>
            <button onclick="sendCommand('limpar')">🧹 Limpar</button>
        </div>
        
        <h3>📊 Estado do Jogo</h3>
        <div>
            <label>Jogador 1: <span id="dist1">0.00</span></label><br>
            <label>Jogador 2: <span id="dist2">0.00</span></label><br>
            <label>Status: <span id="gameStatus">Parado</span></label>
        </div>
        
        <h3>💡 Pista LED</h3>
        <div class="led-track" id="ledTrack"></div>
        
        <h3>📝 Log</h3>
        <div class="log" id="log"></div>
    </div>

    <script>
        let ws = null;
        let ledElements = [];
        
        // Inicializar LEDs
        function initLEDs() {
            const track = document.getElementById('ledTrack');
            track.innerHTML = '';
            ledElements = [];
            
            for (let i = 0; i < 20; i++) {
                const led = document.createElement('div');
                led.className = 'led';
                led.id = 'led-' + i;
                track.appendChild(led);
                ledElements.push(led);
            }
        }
        
        // Conectar WebSocket
        function connect() {
            const ip = window.location.hostname;
            ws = new WebSocket('ws://' + ip + ':81');
            
            ws.onopen = function() {
                updateStatus('Conectado', true);
                log('WebSocket conectado');
            };
            
            ws.onmessage = function(event) {
                const data = JSON.parse(event.data);
                log('Recebido: ' + JSON.stringify(data));
                
                if (data.type === 'state') {
                    updateGameState(data);
                }
            };
            
            ws.onclose = function() {
                updateStatus('Desconectado', false);
                log('WebSocket desconectado');
            };
            
            ws.onerror = function(error) {
                log('Erro: ' + error);
            };
        }
        
        // Desconectar
        function disconnect() {
            if (ws) {
                ws.close();
                ws = null;
            }
        }
        
        // Enviar comando
        function sendCommand(cmd) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(cmd);
                log('Enviado: ' + cmd);
            } else {
                log('Erro: Não conectado');
            }
        }
        
        // Atualizar status
        function updateStatus(text, connected) {
            const status = document.getElementById('status');
            status.textContent = text;
            status.className = 'status ' + (connected ? 'connected' : 'disconnected');
        }
        
        // Atualizar estado do jogo
        function updateGameState(data) {
            document.getElementById('dist1').textContent = data.dist1.toFixed(2);
            document.getElementById('dist2').textContent = data.dist2.toFixed(2);
            document.getElementById('gameStatus').textContent = data.running ? 'Rodando' : 'Parado';
            
            // Atualizar LEDs
            updateLEDs(data);
        }
        
        // Atualizar LEDs
        function updateLEDs(data) {
            // Limpar LEDs
            ledElements.forEach(led => led.style.background = '#333');
            
            // Posicionar jogadores
            const pos1 = Math.floor((data.dist1 / 100) * 19);
            const pos2 = Math.floor((data.dist2 / 100) * 19);
            
            if (pos1 >= 0 && pos1 < 20) {
                ledElements[pos1].style.background = '#ff0000'; // Vermelho
            }
            if (pos2 >= 0 && pos2 < 20) {
                ledElements[pos2].style.background = '#00ff00'; // Verde
            }
        }
        
        // Log
        function log(message) {
            const logDiv = document.getElementById('log');
            const timestamp = new Date().toLocaleTimeString();
            logDiv.innerHTML += '[' + timestamp + '] ' + message + '<br>';
            logDiv.scrollTop = logDiv.scrollHeight;
        }
        
        // Inicializar
        window.onload = function() {
            initLEDs();
            log('Página carregada');
        };
    </script>
</body>
</html>
  )";
}
