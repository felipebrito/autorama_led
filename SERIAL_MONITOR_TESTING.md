# 🔧 Testes via Serial Monitor - Arduino Autorama

## 📋 **Como Usar:**

### **1. Faça Upload** do `arduino/autorama_simple/arduino_autorama_simple.ino`

### **2. Abra o Serial Monitor:**
- Arduino IDE → Ferramentas → Serial Monitor
- **Baud Rate: 115200**
- **Terminador: Nova Linha**

### **3. Comandos Disponíveis:**

| Comando | Função | O que você vê |
|---------|--------|----------------|
| `ajuda` | Mostra todos os comandos | Lista no Serial Monitor |
| `demo` | Animação completa | RGB → Amarelo → Magenta → Ciano |
| `jogador1` | Testa carro vermelho | Carro vermelho se move pela fita |
| `jogador2` | Testa carro verde | Carro verde se move pela fita |
| `simular` | Simula dados do jogo | Envia JSON como se fosse o browser |
| `teste` | Testa todos os LEDs | Cada LED pisca individualmente |
| `limpar` | Limpa todos os LEDs | Fita apaga |
| `parar` | Para modo de teste | Para simulação |

---

## 🧪 **Sequência de Testes Recomendada:**

### **Teste 1: Verificar Hardware**
```
teste
```
- Cada LED deve piscar individualmente
- Depois vermelho, verde, azul em toda a fita

### **Teste 2: Verificar Animações**
```
demo
```
- Sequência de cores por toda a fita
- Deve ser suave e sem falhas

### **Teste 3: Verificar Carros Individuais**
```
jogador1
```
- Carro vermelho se move da esquerda para direita
- Deve ter rastro de 3 LEDs

```
jogador2
```
- Carro verde se move da esquerda para direita
- Deve ter rastro de 3 LEDs

### **Teste 4: Simular Jogo Completo**
```
simular
```
- Envia dados como se fosse o browser
- Deve processar JSON e mostrar carros
- Deve fazer countdown visual

---

## 📊 **O que o Comando `simular` Faz:**

1. **Envia config:** `{"type":"config","maxLed":20,...}`
2. **Inicia jogo:** `{"type":"state","running":1}`
3. **Move carros:** 20 posições com dados simulados
4. **Finaliza jogo:** `{"type":"state","running":0}`

**Resultado esperado:** Countdown 3-2-1 + carros se movendo + fim do jogo

---

## 🔍 **Debug Visual:**

### **Durante Simulação:**
- **LED 0 amarelo** = Modo de teste ativo
- **Countdown:** Vermelho → Amarelo → Verde
- **Carros:** Vermelho (jogador1) + Verde (jogador2)

### **Mensagens no Serial:**
- `"Config recebida via browser"`
- `"State recebido via browser"`
- `"JOGO INICIADO - iniciando countdown visual!"`
- `"Countdown 3 - VERMELHO"`
- `"Countdown 2 - AMARELO"`
- `"Countdown 1 - VERDE"`

---

## ❌ **Se Não Funcionar:**

### **Nenhum comando responde:**
- Verificar baud rate (115200)
- Verificar terminador (Nova Linha)
- Reupload do sketch

### **Comandos respondem mas LEDs não acendem:**
- Verificar alimentação da fita
- Verificar pino 6
- Verificar número de LEDs (NUM_LEDS = 20)

### **Só alguns comandos funcionam:**
- Verificar se todas as funções foram copiadas
- Reupload do sketch

---

## 🎯 **Vantagens deste Sistema:**

- ✅ **Teste individual de cada componente**
- ✅ **Simulação completa do jogo**
- ✅ **Debug visual e textual**
- ✅ **Não depende do browser**
- ✅ **Testa JSON parsing**
- ✅ **Testa renderização dos carros**

---

## 🚀 **Próximo Passo:**

Após testar via Serial Monitor e confirmar que tudo funciona, teste no browser para ver se o problema está na comunicação ou no código Arduino.

**Comandos para testar agora:**
```
ajuda
demo
jogador1
jogador2
simular
```
