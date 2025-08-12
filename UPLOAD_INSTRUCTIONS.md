# 🔧 Instruções para Upload do Sketch Corrigido

## 🚨 PROBLEMA IDENTIFICADO E CORRIGIDO

**Bug crítico encontrado:** O demo do Arduino estava sobrescrevendo os LEDs do jogo porque a lógica estava incorreta.

**O que foi corrigido:**
- Demo agora só roda quando não há dados vivos OU jogo não está ativo
- Adicionados logs detalhados para debug
- Demo é bloqueado quando `haveLiveData=true` E `gameRunning=true`

## 📋 Passos para Upload

### 1. Abrir Arduino IDE
- Baixe e instale o [Arduino IDE](https://www.arduino.cc/en/software) se não tiver
- Ou use o Arduino Web Editor: https://create.arduino.cc/

### 2. Abrir o Sketch Corrigido
- Abra o arquivo: `arduino/arduino_autorama_listener/arduino_autorama_listener.ino`

### 3. Verificar Configurações
- **Board:** Arduino Uno (ou sua placa)
- **Porta:** Selecione a porta USB onde está conectado o Arduino
- **Biblioteca:** Instale `Adafruit_NeoPixel` via Library Manager

### 4. Ajustar Configurações do Hardware
```cpp
#define PIN_LED      6          // Pino da fita WS2812/WS2813
#define PHYS_PIXELS  20         // QUANTIDADE REAL de LEDs na sua fita
```

**IMPORTANTE:** Ajuste `PHYS_PIXELS` para o número real de LEDs que você tem!

### 5. Fazer Upload
- Clique em "Upload" (seta →)
- Aguarde a compilação e upload
- Verifique se não há erros

### 6. Abrir Serial Monitor
- Clique em "Serial Monitor" (lupa)
- Configure para **115200 baud**
- Você deve ver:
```
{"arduino":"ready","leds":20}
{"status":"Arduino iniciado e aguardando comandos"}
```

## 🧪 Teste Após Upload

1. **Recarregue a página** do jogo no navegador
2. **Conecte o Arduino** (clique em "Conectar Arduino")
3. **Clique em "Testar LEDs"** - deve piscar LED central e fazer flash branco
4. **Inicie uma corrida** - agora os LEDs devem acender durante o jogo!

## 🔍 Logs de Debug

Com o sketch corrigido, você verá no Serial Monitor:
```
{"debug":"Estado atualizado - gameRunning:true, haveLiveData:true, lastMsgMs:12345"}
{"debug":"Demo bloqueado - jogo ativo"}
```

## ❌ Se Ainda Não Funcionar

1. **Verifique a alimentação** da fita LED (5V estável)
2. **Confirme o pino** da fita (deve ser o pino 6)
3. **Verifique GND comum** entre Arduino e fita
4. **Teste com menos LEDs** (reduza `PHYS_PIXELS` para 10)

## 📱 Alternativa: Arduino Web Editor

Se preferir não instalar o Arduino IDE:
1. Acesse: https://create.arduino.cc/
2. Faça login/cadastro
3. Crie novo sketch
4. Cole o código corrigido
5. Faça upload via USB

---

**🎯 O bug estava na lógica do demo que sobrescrevia os LEDs do jogo. Agora deve funcionar perfeitamente!**
