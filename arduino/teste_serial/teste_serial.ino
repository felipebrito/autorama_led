/*
 * Teste Serial - Teste de Comunicação Serial
 * Verifica se a comunicação entre Arduino e computador está funcionando
 * Envia e recebe dados para testar a conexão
 */

#include <Adafruit_NeoPixel.h>

#define PIN_LED      6          // Pino da fita LED
#define NUM_LEDS     20         // Quantidade de LEDs na sua fita

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

// Buffer para receber mensagens
String inputString = "";
bool stringComplete = false;

// Contador de mensagens
unsigned long messageCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  strip.begin();
  strip.show(); // Limpar LEDs
  
  Serial.println("=== TESTE SERIAL ===");
  Serial.println("Envie qualquer mensagem para testar");
  Serial.println("Comandos especiais:");
  Serial.println("ping - Responde com pong");
  Serial.println("leds - Testa LEDs");
  Serial.println("clear - Limpa LEDs");
  Serial.println("===================");
  
  // Teste inicial de LEDs
  testeInicialLEDs();
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
    processarMensagem(inputString);
    inputString = "";
    stringComplete = false;
  }
  
  // Demo simples
  static unsigned long lastDemo = 0;
  if (millis() - lastDemo > 3000) {
    runDemo();
    lastDemo = millis();
  }
}

void processarMensagem(String message) {
  message.trim(); // Remover espaços
  
  messageCount++;
  Serial.print("Mensagem #");
  Serial.print(messageCount);
  Serial.print(" recebida: '");
  Serial.print(message);
  Serial.println("'");
  
  // Comandos especiais
  if (message.equalsIgnoreCase("ping")) {
    Serial.println("pong");
    piscarVerde();
  } else if (message.equalsIgnoreCase("leds")) {
    testarLEDs();
  } else if (message.equalsIgnoreCase("clear")) {
    limparLEDs();
  } else {
    // Echo da mensagem
    Serial.print("Echo: ");
    Serial.println(message);
    
    // Confirmação visual
    piscarAzul();
  }
}

void piscarVerde() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 255, 0));
  }
  strip.show();
  delay(200);
  strip.clear();
  strip.show();
}

void piscarAzul() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 255));
  }
  strip.show();
  delay(200);
  strip.clear();
  strip.show();
}

void testarLEDs() {
  Serial.println("Testando LEDs...");
  
  // Sequência de cores
  uint32_t colors[] = {
    strip.Color(255, 0, 0),    // Vermelho
    strip.Color(0, 255, 0),    // Verde
    strip.Color(0, 0, 255),    // Azul
    strip.Color(255, 255, 0),  // Amarelo
  };
  
  for (int color = 0; color < 4; color++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, colors[color]);
    }
    strip.show();
    delay(300);
  }
  
  strip.clear();
  strip.show();
  Serial.println("Teste de LEDs concluído!");
}

void limparLEDs() {
  Serial.println("Limpando LEDs...");
  strip.clear();
  strip.show();
  Serial.println("LEDs limpos!");
}

void testeInicialLEDs() {
  Serial.println("Teste inicial de LEDs...");
  
  // Piscar todos os LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(100, 100, 100));
  }
  strip.show();
  delay(500);
  strip.clear();
  strip.show();
  
  Serial.println("Teste inicial concluído!");
}

void runDemo() {
  static int demoPos = 0;
  strip.clear();
  strip.setPixelColor(demoPos % NUM_LEDS, strip.Color(50, 50, 0));
  strip.show();
  demoPos++;
}
