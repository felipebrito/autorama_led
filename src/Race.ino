#include <FastLED.h>

#define NUM_LEDS 200
#define LED_PIN 4
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// Cores dos carros (ajustadas para ordem GRB correta)
#define COLOR1 0x00FF00  // VERMELHO (carro 1) - GRB: Green=0xFF, Red=0x00, Blue=0x00
#define COLOR2 0xFF0000  // VERDE (carro 2) - GRB: Green=0x00, Red=0xFF, Blue=0x00
#define COLOR3 0x0000FF  // AZUL (carro 3) - GRB: Green=0x00, Red=0x00, Blue=0xFF
#define COLOR4 0xFFFF00  // AMARELO (carro 4) - GRB: Green=0xFF, Red=0xFF, Blue=0x00

// Variáveis do jogo
bool race_started = false;
float car_speeds[4] = {0, 0, 0, 0};
float car_positions[4] = {0, 0, 0, 0};
int car_laps[4] = {0, 0, 0, 0}; // Voltas de cada carro
int winner = -1; // Vencedor (-1 = sem vencedor)
const int TOTAL_LAPS = 5; // Corrida termina em 5 voltas

void setup() {
  Serial.begin(115200);
  Serial.println("🚀 OLR ESP32 - JOGO COMPLETO");
  
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(64);
  
  Serial.println("💡 LEDs inicializados");
  
  // Mostrar cores definidas
  Serial.println("🎨 Cores dos carros:");
  Serial.print("  Carro 1 (Vermelho): 0x");
  Serial.println(COLOR1, HEX);
  Serial.print("  Carro 2 (Verde): 0x");
  Serial.println(COLOR2, HEX);
  Serial.print("  Carro 3 (Azul): 0x");
  Serial.println(COLOR3, HEX);
  Serial.print("  Carro 4 (Amarelo): 0x");
  Serial.println(COLOR4, HEX);
  
  // Teste inicial de cores
  Serial.println("🧪 Teste inicial de cores...");
  
  // Teste vermelho
  Serial.println("🔴 Teste: VERMELHO");
  fill_solid(leds, NUM_LEDS, COLOR1);
  FastLED.show();
  delay(1000);
  
  // Teste verde
  Serial.println("🟢 Teste: VERDE");
  fill_solid(leds, NUM_LEDS, COLOR2);
  FastLED.show();
  delay(1000);
  
  // Teste azul
  Serial.println("🔵 Teste: AZUL");
  fill_solid(leds, NUM_LEDS, COLOR3);
  FastLED.show();
  delay(1000);
  
  // Teste amarelo
  Serial.println("🟡 Teste: AMARELO");
  fill_solid(leds, NUM_LEDS, COLOR4);
  FastLED.show();
  delay(1000);
  
  // Limpar
  Serial.println("⚫ LEDs apagados");
  FastLED.clear();
  FastLED.show();
  
  Serial.println("✅ Setup concluído!");
  Serial.println("🎯 Corrida termina em 5 voltas!");
  Serial.println("📋 Comandos disponíveis:");
  Serial.println("  g = GO (iniciar corrida)");
  Serial.println("  a = Acelerar Carro 1 (Vermelho)");
  Serial.println("  2 = Acelerar Carro 2 (Verde)");
  Serial.println("  d = Acelerar Carro 3 (Azul)");
  Serial.println("  f = Acelerar Carro 4 (Amarelo)");
  Serial.println("  r = RESET");
  Serial.println("  t = TESTE");
  Serial.println("  s = STATUS");
  Serial.println("  c = CLEAR (limpar fita)");
  Serial.println("  v = VIEW (ver todos os carros)");
}

void loop() {
  // Processar comandos seriais
  if (Serial.available()) {
    char cmd = Serial.read();
    processCommand(cmd);
  }
  
  // Atualizar jogo se corrida iniciada
  if (race_started) {
    updateGame();
    drawGame();
    FastLED.show();
  }
  
  delay(50); // 20 FPS
}

void processCommand(char cmd) {
  Serial.print("📡 Comando: ");
  Serial.println(cmd);
  
  switch(cmd) {
    case 'g': case 'G':
      Serial.println("🏁 GO - Iniciando corrida!");
      race_started = true;
      winner = -1; // Resetar vencedor
      // Resetar posições, velocidades e voltas
      for (int i = 0; i < 4; i++) {
        car_positions[i] = 0;
        car_speeds[i] = 0;
        car_laps[i] = 0;
      }
      Serial.print("🎯 Meta: ");
      Serial.print(TOTAL_LAPS);
      Serial.println(" voltas para vencer!");
      break;
      
    case 'a': case 'A':
      Serial.println("🚗 Carro 1 (Vermelho) acelerando!");
      if (race_started) {
        car_speeds[0] = min(car_speeds[0] + 0.5, 5.0);
        Serial.print("  Nova velocidade: ");
        Serial.println(car_speeds[0]);
      }
      break;
      
    case '2':
      Serial.println("🚗 Carro 2 (Verde) acelerando!");
      if (race_started) {
        car_speeds[1] = min(car_speeds[1] + 0.5, 5.0);
        Serial.print("  Nova velocidade: ");
        Serial.println(car_speeds[1]);
      }
      break;
      
    case 'd': case 'D':
      Serial.println("🚗 Carro 3 (Azul) acelerando!");
      if (race_started) {
        car_speeds[2] = min(car_speeds[2] + 0.5, 5.0);
        Serial.print("  Nova velocidade: ");
        Serial.println(car_speeds[2]);
      }
      break;
      
    case 'f': case 'F':
      Serial.println("🚗 Carro 4 (Amarelo) acelerando!");
      if (race_started) {
        car_speeds[3] = min(car_speeds[3] + 0.5, 5.0);
        Serial.print("  Nova velocidade: ");
        Serial.println(car_speeds[3]);
      }
      break;
      
    case 'r': case 'R':
      Serial.println("🔄 RESET - Parando corrida!");
      race_started = false;
      winner = -1; // Resetar vencedor
      // Limpar todos os LEDs
      FastLED.clear();
      FastLED.show();
      // Resetar posições, velocidades e voltas
      for (int i = 0; i < 4; i++) {
        car_positions[i] = 0;
        car_speeds[i] = 0;
        car_laps[i] = 0;
      }
      break;
      
    case 't': case 'T':
      Serial.println("🧪 TESTE - Modo teste!");
      testMode();
      break;
      
    case 's': case 'S':
      Serial.println("📊 STATUS:");
      Serial.print("  Corrida: ");
      Serial.println(race_started ? "ATIVA" : "INATIVA");
      if (winner != -1) {
        Serial.print("🏆 VENCEDOR: Carro ");
        Serial.println(winner + 1);
      }
      for (int i = 0; i < 4; i++) {
        Serial.print("  Carro ");
        Serial.print(i + 1);
        Serial.print(": Vel=");
        Serial.print(car_speeds[i]);
        Serial.print(", Pos=");
        Serial.print(car_positions[i]);
        Serial.print(", Voltas=");
        Serial.print(car_laps[i]);
        Serial.print("/");
        Serial.println(TOTAL_LAPS);
      }
      break;
      
    case 'c': case 'C':
      clearStrip();
      break;
      
    case 'v': case 'V':
      showAllCars();
      break;
      
    default:
      Serial.print("❓ Comando desconhecido: ");
      Serial.println(cmd);
      break;
  }
}

void updateGame() {
  // Verificar se já há um vencedor
  if (winner != -1) {
    return; // Jogo acabou
  }
  
  for (int i = 0; i < 4; i++) {
    if (car_speeds[i] > 0) {
      // Fricção
      car_speeds[i] = max(car_speeds[i] - 0.1, 0.0);
      
      // Atualizar posição
      car_positions[i] += car_speeds[i];
      
      // Volta completa
      if (car_positions[i] >= NUM_LEDS) {
        car_positions[i] = 0;
        car_laps[i]++; // Incrementar volta
        
        Serial.print("🏁 Carro ");
        Serial.print(i + 1);
        Serial.print(" completou volta ");
        Serial.print(car_laps[i]);
        Serial.print("/");
        Serial.print(TOTAL_LAPS);
        Serial.print(" - Velocidade: ");
        Serial.println(car_speeds[i]);
        
        // Verificar se ganhou
        if (car_laps[i] >= TOTAL_LAPS) {
          winner = i;
          race_started = false;
          Serial.println("🎉🎉🎉 CORRIDA TERMINOU! 🎉🎉🎉");
          Serial.print("🏆 VENCEDOR: Carro ");
          Serial.print(i + 1);
          Serial.println("!");
          
          // Mostrar vitória na fita
          showVictory(i);
          return;
        }
      }
    }
  }
}

void drawGame() {
  // NÃO limpar todos os LEDs - permitir sobreposição
  // FastLED.clear(); // REMOVIDO!
  
  for (int i = 0; i < 4; i++) {
    // MOSTRAR TODOS OS CARROS, mesmo parados!
    // if (car_speeds[i] > 0) { // REMOVIDO!
    
    int pos = (int)car_positions[i] % NUM_LEDS;
    if (pos >= 0 && pos < NUM_LEDS) {
      uint32_t color;
      switch(i) {
        case 0: color = COLOR1; break; // Vermelho
        case 1: color = COLOR2; break; // Verde
        case 2: color = COLOR3; break; // Azul
        case 3: color = COLOR4; break; // Amarelo
      }
      
      // Carro principal - SOMAR cores se sobreposto
      if (leds[pos] != CRGB(0, 0, 0)) {
        // Se já há uma cor, somar (efeito de sobreposição)
        leds[pos] += color;
      } else {
        // Se não há cor, definir diretamente
        leds[pos] = color;
      }
      
      // Rastro sutil - também somar cores
      for (int j = 1; j <= 2; j++) {
        if (pos - j >= 0) {
          CRGB rastro_color = color;
          rastro_color.fadeToBlackBy(128); // Rastro mais sutil
          
          if (leds[pos - j] != CRGB(0, 0, 0)) {
            // Somar com cor existente
            leds[pos - j] += rastro_color;
          } else {
            // Definir diretamente
            leds[pos - j] = rastro_color;
          }
        }
      }
    }
    // } // REMOVIDO!
  }
  
  // Aplicar fade gradual para todos os LEDs (efeito de movimento)
  for (int i = 0; i < NUM_LEDS; i++) {
    if (leds[i] != CRGB(0, 0, 0)) {
      leds[i].fadeToBlackBy(32); // Fade sutil para todos
    }
  }
}

void testMode() {
  Serial.println("🧪 Iniciando modo teste...");
  
  // Sequência de cores dos carros
  for (int i = 0; i < 4; i++) {
    uint32_t color;
    switch(i) {
      case 0: color = COLOR1; Serial.println("🔴 Carro 1 - Vermelho"); break;
      case 1: color = COLOR2; Serial.println("🟢 Carro 2 - Verde"); break;
      case 2: color = COLOR3; Serial.println("🔵 Carro 3 - Azul"); break;
      case 3: color = COLOR4; Serial.println("🟡 Carro 4 - Amarelo"); break;
    }
    
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
        delay(1000);
  }
  
  // Limpar
  FastLED.clear();
  FastLED.show();
  Serial.println("✅ Teste concluído!");
}

// Função para limpar a fita (comando 'c')
void clearStrip() {
  Serial.println("🧹 Limpando fita LED...");
  FastLED.clear();
  FastLED.show();
}

// Função para mostrar vitória
void showVictory(int carro_vencedor) {
  uint32_t color;
  switch(carro_vencedor) {
    case 0: color = COLOR1; break; // Vermelho
    case 1: color = COLOR2; break; // Verde
    case 2: color = COLOR3; break; // Azul
    case 3: color = COLOR4; break; // Amarelo
  }
  
  // Piscar toda a fita com a cor do vencedor
  for (int flash = 0; flash < 10; flash++) {
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
    delay(200);
    FastLED.clear();
    FastLED.show();
    delay(200);
  }
  
  // Manter a fita acesa com a cor do vencedor
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
}

// Função para mostrar todos os carros
void showAllCars() {
  Serial.println("👀 Mostrando todos os carros na fita...");
  
  // Limpar primeiro
  FastLED.clear();
  
  // Mostrar cada carro em sua posição
  for (int i = 0; i < 4; i++) {
    int pos = (int)car_positions[i] % NUM_LEDS;
    if (pos >= 0 && pos < NUM_LEDS) {
      uint32_t color;
      switch(i) {
        case 0: color = COLOR1; break; // Vermelho
        case 1: color = COLOR2; break; // Verde
        case 2: color = COLOR3; break; // Azul
        case 3: color = COLOR4; break; // Amarelo
      }
      
      // Carro principal
      leds[pos] = color;
      
      // Rastro sutil
      for (int j = 1; j <= 2; j++) {
        if (pos - j >= 0) {
          CRGB rastro_color = color;
          rastro_color.fadeToBlackBy(128);
          leds[pos - j] = rastro_color;
        }
      }
      
      Serial.print("  Carro ");
      Serial.print(i + 1);
      Serial.print(" na posição ");
      Serial.println(pos);
    }
  }
  
  FastLED.show();
  Serial.println("✅ Todos os carros exibidos!");
}

