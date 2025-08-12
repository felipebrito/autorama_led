# 🎮 Arduino - Autorama LED

## 📁 **Estrutura Organizada (Nova)**

### **🎯 Sketch Principal:**
- **`autorama_principal/autorama_principal.ino`** ← **USE ESTE PARA O JOGO!**
  - Sketch principal com todos os testes
  - Comandos: demo, jogador1, jogador2, simular, ajuda, limpar, teste
  - Sistema completo de debug visual

### **🧪 Sketches de Teste:**
- **`teste_leds/teste_leds.ino`** ← Teste básico de LEDs
  - Comandos: r, g, b, c, t
  - Para verificar se o hardware funciona

- **`teste_serial/teste_serial.ino`** ← Teste de comunicação
  - Comandos: ping, leds, clear
  - Para verificar se a comunicação funciona

### **📚 Sketches de Backup:**
- **`autorama_listener/autorama_listener.ino`** ← Versão antiga
  - Mantido como referência
  - Não usar para testes

---

## 🚀 **COMO USAR:**

### **1. Para o JOGO (Recomendado):**
```
arduino/autorama_principal/autorama_principal.ino
```

### **2. Para testar LEDs:**
```
arduino/teste_leds/teste_leds.ino
```

### **3. Para testar comunicação:**
```
arduino/teste_serial/teste_serial.ino
```

---

## 🔧 **Comandos do Sketch Principal:**

| Comando | Função |
|---------|--------|
| `ajuda` | Mostra todos os comandos |
| `demo` | Animação completa de cores |
| `jogador1` | Testa carro vermelho |
| `jogador2` | Testa carro verde |
| `simular` | Simula dados do jogo |
| `limpar` | Limpa todos os LEDs |
| `teste` | Testa todos os LEDs |
| `parar` | Para modo de teste |

---

## 📋 **Sequência de Testes:**

1. **Upload** do `autorama_principal.ino`
2. **Serial Monitor** (115200 baud)
3. **`ajuda`** → Ver comandos disponíveis
4. **`teste`** → Verificar se LEDs funcionam
5. **`demo`** → Verificar se animações funcionam
6. **`simular`** → Teste completo do jogo

---

## ❌ **Estrutura Antiga (Remover):**

- ~~`arduino/autorama_simple/`~~ ← **DUPLICADO!**
- ~~`arduino/led_test_simple/`~~ ← **RENOMEADO!**
- ~~`arduino/serial_test/`~~ ← **RENOMEADO!**
- ~~`arduino/arduino_autorama_listener/`~~ ← **BACKUP!**

---

## 🎯 **Vantagens da Nova Estrutura:**

- ✅ **Nomes claros em português**
- ✅ **Sem duplicação de arquivos**
- ✅ **Organização lógica**
- ✅ **Fácil de encontrar**
- ✅ **Sketch principal bem identificado**

**Use sempre o `autorama_principal.ino` para o jogo! 🚀**
