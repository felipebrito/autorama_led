/*
 * Autorama Principal - Sketch Principal do Jogo
 * Versão com Testes via Serial Monitor
 * Comandos de teste: demo, jogador1, jogador2, simular, ajuda
 * Animações presetadas para testar cada componente
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

// Modo de teste
bool testMode = false;
unsigned long testStartTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  strip.begin();
  strip.show(); // Limpar LEDs
  
  // Teste visual de inicialização
  testLEDs();
  
  // Mostrar comandos disponíveis
  showHelp();
}

void loop() {
  // PRIORIDADE MÁXIMA: Ler mensagens do serial
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    if (inChar == '\n') {
      stringComplete = true;
      Serial.println("DEBUG: \\n detectado, mensagem completa!");
    } else {
      inputString += inChar;
      Serial.print("DEBUG: Caractere recebido: '");
      Serial.print(inChar);
      Serial.print("' (ASCII: ");
      Serial.print((int)inChar);
      Serial.println(")");
    }
  }
  
  // Processar mensagem completa IMEDIATAMENTE
  if (stringComplete) {
    Serial.println("DEBUG: Processando mensagem completa...");
    processMessage(inputString);
    inputString = "";
    stringComplete = false;
    Serial.println("DEBUG: Mensagem processada, buffer limpo!");
  }
  
  // Modo de teste ativo
  if (testMode) {
    runTestMode();
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
  
  // Demo simples se não receber mensagens (REDUZIR FREQUÊNCIA)
  static unsigned long lastDemo = 0;
  if (millis() - lastDemo > 10000 && !testMode) { // Aumentar de 2000 para 10000ms
    runDemo();
    lastDemo = millis();
  }
}

void processMessage(String message) {
  message.trim(); // Remover espaços
  
  Serial.print("Processando mensagem: '");
  Serial.print(message);
  Serial.println("'");
  
  // DEBUG DETALHADO: Mostrar cada caractere individualmente
  Serial.print("DEBUG: Mensagem recebida [");
  for (int i = 0; i < message.length(); i++) {
    char c = message.charAt(i);
    if (c >= 32 && c <= 126) {
      Serial.print(c); // Caractere visível
    } else {
      Serial.print("\\x"); // Caractere invisível
      if (c < 16) Serial.print("0");
      Serial.print(c, HEX);
    }
  }
  Serial.println("]");
  
  Serial.print("DEBUG: Tamanho da mensagem: ");
  Serial.println(message.length());
  
  Serial.print("DEBUG: Primeiro caractere (ASCII): ");
  if (message.length() > 0) {
    Serial.println((int)message.charAt(0));
  } else {
    Serial.println("MENSAGEM VAZIA!");
  }
  
  // DEBUG: Verificar se é JSON válido
  if (message.startsWith("{") && message.endsWith("}")) {
    Serial.println("✓ Mensagem parece ser JSON válido");
  } else {
    Serial.println("✗ Mensagem não parece ser JSON válido");
  }
  
  // Comandos de teste via Serial Monitor
  if (message.equalsIgnoreCase("demo")) {
    Serial.println("Comando DEMO detectado");
    runDemoAnimation();
  } else if (message.equalsIgnoreCase("jogador1")) {
    Serial.println("✓ Comando JOGADOR1 detectado");
    testJogador1();
  } else if (message.equalsIgnoreCase("jogador2")) {
    Serial.println("✓ Comando JOGADOR2 detectado");
    testJogador2();
  } else if (message.equalsIgnoreCase("simular")) {
    Serial.println("✓ Comando SIMULAR detectado");
    startSimulation();
  } else if (message.equalsIgnoreCase("parar")) {
    Serial.println("✓ Comando PARAR detectado");
    stopTestMode();
  } else if (message.equalsIgnoreCase("ajuda") || message.equalsIgnoreCase("help")) {
    Serial.println("✓ Comando AJUDA detectado");
    showHelp();
  } else if (message.equalsIgnoreCase("limpar")) {
    Serial.println("✓ Comando LIMPAR detectado");
    Serial.println("Executando comando LIMPAR...");
    strip.clear();
    strip.show();
    Serial.println("LEDs limpos com sucesso!");
    
    // Confirmação visual - piscar azul 2x
    for (int flash = 0; flash < 2; flash++) {
      for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(0, 0, 100));
      }
      strip.show();
      delay(100);
      strip.clear();
      strip.show();
      delay(100);
    }
  } else if (message.equalsIgnoreCase("teste")) {
    Serial.println("✓ Comando TESTE detectado");
    testAllLEDs();
  } else if (message.indexOf("\"type\":\"config\"") >= 0) {
    Serial.println("✓ Comando CONFIG detectado");
    Serial.println("Chamando handleConfig...");
    handleConfig(message);
    Serial.println("✓ handleConfig concluído");
  } else if (message.indexOf("\"type\":\"effect\"") >= 0) {
    Serial.println("✓ Comando EFFECT detectado");
    Serial.println("Chamando handleEffect...");
    handleEffect(message);
    Serial.println("✓ handleEffect concluído");
  } else if (message.indexOf("\"type\":\"ping\"") >= 0) {
    Serial.println("✓ Comando PING detectado");
    Serial.println("Chamando handlePing...");
    handlePing(message);
    Serial.println("✓ handlePing concluído");
  } else {
    Serial.print("✗ Comando não reconhecido: '");
    Serial.print(message);
    Serial.println("'");
    Serial.println("Digite 'ajuda' para ver comandos disponíveis");
  }
}

void showHelp() {
  Serial.println("\n=== COMANDOS DE TESTE ===");
  Serial.println("demo      - Animação demo completa");
  Serial.println("jogador1  - Testa carro vermelho");
  Serial.println("jogador2  - Testa carro verde");
  Serial.println("simular   - Simula dados do jogo");
  Serial.println("parar     - Para modo de teste");
  Serial.println("limpar    - Limpa todos os LEDs");
  Serial.println("teste     - Testa todos os LEDs");
  Serial.println("ajuda     - Mostra esta ajuda");
  Serial.println("========================\n");
}

void runDemoAnimation() {
  Serial.println("Executando animação DEMO...");
  
  // Sequência de cores
  uint32_t colors[] = {
    strip.Color(255, 0, 0),    // Vermelho
    strip.Color(0, 255, 0),    // Verde
    strip.Color(0, 0, 255),    // Azul
    strip.Color(255, 255, 0),  // Amarelo
    strip.Color(255, 0, 255),  // Magenta
    strip.Color(0, 255, 255)   // Ciano
  };
  
  for (int color = 0; color < 6; color++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, colors[color]);
      strip.show();
      delay(50);
    }
    delay(200);
  }
  
  strip.clear();
  strip.show();
  Serial.println("Demo concluído!");
}

void testJogador1() {
  Serial.println("Testando JOGADOR 1 (vermelho)...");
  
  // Carro vermelho se move pela fita
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
  Serial.println("Teste Jogador 1 concluído!");
}

void testJogador2() {
  Serial.println("Testando JOGADOR 2 (verde)...");
  
  // Carro verde se move pela fita
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
  Serial.println("Teste Jogador 2 concluído!");
}

void startSimulation() {
  Serial.println("Iniciando SIMULAÇÃO de dados do jogo...");
  Serial.println("Enviando dados como se fosse o browser...");
  
  testMode = true;
  testStartTime = millis();
  
  // Indicador visual de que está em modo de teste
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 255, 0)); // Amarelo
  }
  strip.show();
  delay(500);
  strip.clear();
  strip.show();
  
  Serial.println("1. Enviando CONFIG...");
  processMessage("{\"type\":\"config\",\"maxLed\":20,\"loopMax\":5,\"acel\":0.2,\"kf\":0.015,\"kg\":0.003,\"tail\":3}");
  delay(500);
  
  Serial.println("2. Iniciando JOGO...");
  processMessage("{\"type\":\"state\",\"dist1\":0,\"dist2\":0,\"speed1\":0,\"speed2\":0,\"loop1\":0,\"loop2\":0,\"leader\":0,\"running\":1}");
  delay(1000);
  
  Serial.println("3. Simulando movimento dos carros...");
  // Simular movimento dos carros
  for (int step = 0; step < 20; step++) {
    float d1 = step * 1.5;
    float d2 = step * 1.2;
    
    String stateMsg = "{\"type\":\"state\",\"dist1\":" + String(d1) + ",\"dist2\":" + String(d2) + ",\"speed1\":1.5,\"speed2\":1.2,\"loop1\":" + (step/5) + ",\"loop2\":" + (step/6) + ",\"leader\":" + (d1 > d2 ? 1 : 2) + ",\"running\":1}";
    processMessage(stateMsg);
    
    Serial.print("Step "); Serial.print(step); Serial.print(": d1="); Serial.print(d1); Serial.print(", d2="); Serial.println(d2);
    delay(300);
  }
  
  Serial.println("4. Finalizando jogo...");
  processMessage("{\"type\":\"state\",\"dist1\":30,\"dist2\":24,\"speed1\":0,\"speed2\":0,\"loop1\":1,\"loop2\":0,\"leader\":1,\"running\":0}");
  
  // Indicador visual de conclusão
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 255, 0)); // Verde
  }
  strip.show();
  delay(500);
  strip.clear();
  strip.show();
  
  Serial.println("Simulação concluída!");
  testMode = false;
}

void runTestMode() {
  // Durante o modo de teste, não executar outras animações
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 100) {
    lastUpdate = millis();
    
    // Mostrar indicador visual de que está em modo de teste
    strip.setPixelColor(0, strip.Color(255, 255, 0)); // Amarelo no primeiro LED
    strip.show();
  }
}

void stopTestMode() {
  Serial.println("Parando modo de teste...");
  testMode = false;
  strip.clear();
  strip.show();
  Serial.println("Modo de teste parado");
}

void testAllLEDs() {
  Serial.println("Testando todos os LEDs...");
  
  // Teste individual de cada LED
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.clear();
    strip.setPixelColor(i, strip.Color(255, 255, 255));
    strip.show();
    delay(100);
  }
  
  // Teste de cores
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }
  strip.show();
  delay(500);
  
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 255, 0));
  }
  strip.show();
  delay(500);
  
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 255));
  }
  strip.show();
  delay(500);
  
  strip.clear();
  strip.show();
  Serial.println("Teste de LEDs concluído!");
}

void handleConfig(String message) {
  Serial.println("Config recebida via browser");
  Serial.println("Iniciando animação visual...");
  
  // PADRÃO VISUAL: PISCAR VERDE 2x (config recebida)
  for (int flash = 0; flash < 2; flash++) {
    Serial.print("Flash "); Serial.print(flash + 1); Serial.println("/2");
    
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, strip.Color(0, 255, 0));
    }
    strip.show();
    Serial.println("✓ LEDs verdes acesos");
    delay(150);
    
    strip.clear();
    strip.show();
    Serial.println("✓ LEDs limpos");
    delay(150);
  }
  
  Serial.println("✓ Animação de config concluída!");
}

void handleState(String message) {
  Serial.println("State recebido via browser");
  
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
      Serial.println("JOGO INICIADO - iniciando countdown visual!");
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
  
  Serial.print("dist1: "); Serial.print(dist1);
  Serial.print(", dist2: "); Serial.print(dist2);
  Serial.print(", running: "); Serial.println(gameRunning ? "true" : "false");
}

void handleEffect(String message) {
  Serial.println("Effect recebido via browser");
  Serial.println("Iniciando flash branco...");
  
  // PADRÃO VISUAL: FLASH BRANCO RÁPIDO (effect recebido)
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 255, 255));
  }
  strip.show();
  Serial.println("✓ Todos os LEDs brancos acesos");
  delay(80);
  
  strip.clear();
  strip.show();
  Serial.println("✓ LEDs limpos");
  delay(80);
  
  strip.show();
  Serial.println("✓ LEDs brancos novamente");
  delay(80);
  
  strip.clear();
  strip.show();
  Serial.println("✓ LEDs limpos finais");
  Serial.println("✓ Flash branco concluído!");
}

void handlePing(String message) {
  Serial.println("Ping recebido via browser");
  Serial.println("Iniciando animação de ping...");
  
  // PADRÃO VISUAL: PISCAR AZUL NO CENTRO (ping recebido)
  int center = NUM_LEDS / 2;
  Serial.print("LED central: "); Serial.println(center);
  
  strip.setPixelColor(center, strip.Color(0, 0, 255));
  strip.show();
  Serial.println("✓ LED azul aceso no centro");
  delay(200);
  
  strip.setPixelColor(center, strip.Color(0, 0, 0));
  strip.show();
  Serial.println("✓ LED central apagado");
  Serial.println("✓ Animação de ping concluída!");
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
        Serial.println("Countdown 3 - VERMELHO");
        break;
      case 1: // Amarelo
        color = strip.Color(255, 255, 0);
        Serial.println("Countdown 2 - AMARELO");
        break;
      case 2: // Verde
        color = strip.Color(0, 255, 0);
        Serial.println("Countdown 1 - VERDE");
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
