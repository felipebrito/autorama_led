# 🏁 Autorama 4 Jogadores - ESP32 LED Race Game

<div align="center">

![ESP32](https://img.shields.io/badge/ESP32-ESP32--DevModule-blue)
![FastLED](https://img.shields.io/badge/FastLED-3.10.1-green)
![PlatformIO](https://img.shields.io/badge/PlatformIO-6.12.0-orange)
![Python](https://img.shields.io/badge/Python-3.8+-yellow)

**🎮 Jogo de corrida de LED para 4 jogadores com ESP32 e fita WS2812B**

[🚀 Começar](#-como-começar) • [🎯 Funcionalidades](#-funcionalidades) • [🔧 Hardware](#-hardware) • [📱 Interface](#-interface)

</div>

---

## 🌟 Sobre o Projeto

**Autorama 4 Jogadores** é um jogo de corrida competitivo onde 4 jogadores competem em uma pista de LED, controlando seus carros através de comandos seriais. O projeto foi desenvolvido para ESP32 com fita LED WS2812B, oferecendo uma experiência visual única e interativa.

### ✨ Características Principais

- 🚗 **4 carros com cores distintas** (Vermelho, Verde, Azul, Amarelo)
- 🏁 **Sistema de voltas** (5 voltas para vencer)
- 💫 **Efeitos visuais** com rastros e sobreposição de cores
- 📱 **Interface web moderna** com Neo Brutalism UI
- 🔧 **Controle via serial** para desenvolvimento e teste
- ⚡ **Física realista** com aceleração e fricção

---

## 🎯 Funcionalidades

### 🎮 Sistema de Jogo
- **Corrida automática** com contador de voltas
- **Declaração de vencedor** quando completar 5 voltas
- **Celebração visual** na fita LED
- **Reset completo** para nova corrida

### 🎨 Efeitos Visuais
- **Carros sempre visíveis** na fita LED
- **Rastros sutis** para cada carro
- **Sobreposição de cores** quando carros se cruzam
- **Fade gradual** para movimento suave

### 📊 Controles e Status
- **Comandos seriais** para cada carro
- **Status em tempo real** com posições e voltas
- **Interface web responsiva** para controle remoto
- **Logs detalhados** para debugging

---

## 🔧 Hardware

### 📋 Componentes Necessários

| Componente | Especificação | Quantidade |
|------------|---------------|------------|
| **ESP32** | ESP32-DevModule | 1 |
| **Fita LED** | WS2812B 5V | 10m (200 LEDs) |
| **Fonte** | 5V 3A | 1 |
| **Cabo USB** | Micro USB | 1 |
| **Protoboard** | 400 pontos | 1 |
| **Jumpers** | Macho-Macho | 10+ |

### 🔌 Conexões

```
ESP32 Pin D4 → Fita LED DATA
ESP32 GND → Fita LED GND
ESP32 5V → Fita LED VCC
```

### ⚠️ Observações Importantes

- **Alimentação:** Use fonte 5V 3A para fita de 200 LEDs
- **Proteção:** Capacitor 1000µF entre VCC e GND
- **Resistência:** 330Ω no pino DATA (opcional)

---

## 📱 Interface

### 🌐 Interface Web

O projeto inclui uma interface web moderna construída com:

- **Neo Brutalism UI** - Design moderno e responsivo
- **Tailwind CSS** - Estilização profissional
- **JavaScript ES6** - Funcionalidades interativas
- **Python Flask** - Servidor backend robusto

### 🎮 Controles Web

- **Botões de aceleração** para cada carro
- **Sliders de configuração** para física do jogo
- **Visualização da pista** em tempo real
- **Tabela de pontuação** com voltas e tempos
- **Log de comandos** para debugging

### 📱 Interface Python

- **Tkinter GUI** para configuração local
- **Controle via terminal** para desenvolvimento
- **Configuração de parâmetros** do jogo
- **Monitoramento em tempo real**

---

## 🚀 Como Começar

### 📋 Pré-requisitos

- **Arduino IDE** ou **PlatformIO**
- **Python 3.8+**
- **ESP32** conectado via USB
- **Fita LED WS2812B**

### 🔧 Instalação

1. **Clone o repositório:**
```bash
git clone https://github.com/felipebrito/autorama_4jogadores.git
cd autorama_4jogadores
```

2. **Instale as dependências Python:**
```bash
pip install pyserial flask
```

3. **Abra o projeto no PlatformIO:**
```bash
pio run -t upload
```

### 🎮 Primeiros Passos

1. **Conecte o ESP32** via USB
2. **Faça upload do firmware** via PlatformIO
3. **Abra o monitor serial:**
```bash
pio device monitor --baud 115200
```

4. **Teste os comandos básicos:**
```bash
g    # Iniciar corrida
a    # Acelerar Carro 1 (Vermelho)
2    # Acelerar Carro 2 (Verde)
d    # Acelerar Carro 3 (Azul)
f    # Acelerar Carro 4 (Amarelo)
s    # Ver status
r    # Reset
```

---

## 📖 Comandos Disponíveis

### 🎮 Controles de Jogo

| Comando | Ação | Descrição |
|---------|------|-----------|
| `g` / `G` | **GO** | Inicia a corrida |
| `a` / `A` | **Carro 1** | Acelera carro vermelho |
| `2` | **Carro 2** | Acelera carro verde |
| `d` / `D` | **Carro 3** | Acelera carro azul |
| `f` / `F` | **Carro 4** | Acelera carro amarelo |
| `r` / `R` | **RESET** | Para corrida e reseta tudo |
| `s` / `S` | **STATUS** | Mostra status completo |
| `t` / `T` | **TESTE** | Modo teste de cores |
| `c` / `C` | **CLEAR** | Limpa a fita LED |
| `v` / `V` | **VIEW** | Mostra todos os carros |

### ⚙️ Configurações

- **Voltas para vencer:** 5
- **Velocidade máxima:** 5.0
- **Taxa de aceleração:** 0.2
- **Fricção:** 0.1 por frame
- **FPS:** 20 (delay 50ms)

---

## 🎨 Cores dos Carros

### 🌈 Sistema de Cores GRB

| Carro | Cor | Código Hex | Descrição |
|-------|-----|------------|-----------|
| **Carro 1** | 🔴 Vermelho | `0x00FF00` | GRB: Green=0xFF, Red=0x00, Blue=0x00 |
| **Carro 2** | 🟢 Verde | `0xFF0000` | GRB: Green=0x00, Red=0xFF, Blue=0x00 |
| **Carro 3** | 🔵 Azul | `0x0000FF` | GRB: Green=0x00, Red=0x00, Blue=0xFF |
| **Carro 4** | 🟡 Amarelo | `0xFFFF00` | GRB: Green=0xFF, Red=0xFF, Blue=0x00 |

### ⚠️ Nota sobre Cores

O ESP32 usa **ordem GRB** para WS2812B, diferente do RGB padrão. As cores são definidas considerando esta diferença.

---

## 🔧 Configuração Avançada

### 📁 Estrutura do Projeto

```
autorama_4jogadores/
├── src/
│   └── Race.ino          # Firmware principal ESP32
├── web_interface.html    # Interface web
├── web_server_robust.py  # Servidor Python
├── olr_gui_config.py     # Interface Tkinter
├── platformio.ini        # Configuração PlatformIO
├── olr-arduino/          # Projeto base OLR-Arduino
└── README.md             # Esta documentação
```

### ⚙️ Parâmetros Configuráveis

```cpp
#define NUM_LEDS 200           // Número de LEDs na fita
#define LED_PIN 4              // Pino de dados
#define LED_TYPE WS2812B       // Tipo de LED
#define COLOR_ORDER GRB        // Ordem das cores
#define TOTAL_LAPS 5           // Voltas para vencer
```

### 🎮 Física do Jogo

- **Aceleração:** Incremento por comando
- **Fricção:** Redução gradual da velocidade
- **Posição:** Atualização baseada na velocidade
- **Voltas:** Contagem automática ao completar pista

---

## 🚀 Interface Web

### 🌐 Iniciar Servidor

```bash
python3 web_server_robust.py
```

### 📱 Acessar Interface

Abra o navegador em: `http://localhost:8000`

### 🎮 Funcionalidades Web

- **Controle em tempo real** dos carros
- **Configuração de parâmetros** via sliders
- **Visualização da pista** com posições dos carros
- **Tabela de pontuação** com voltas e tempos
- **Log de comandos** para debugging

---

## 🐛 Troubleshooting

### ❌ Problemas Comuns

| Problema | Solução |
|----------|---------|
| **ESP32 não conecta** | Verificar cabo USB e drivers |
| **LEDs não acendem** | Verificar conexões e alimentação |
| **Cores incorretas** | Confirmar ordem GRB vs RGB |
| **Carros não aparecem** | Verificar comandos seriais |
| **Interface web não carrega** | Verificar servidor Python |

### 🔍 Debug

1. **Monitor Serial:** Verificar logs do ESP32
2. **Interface Web:** Logs no console do navegador
3. **Python:** Logs do servidor web
4. **Hardware:** Verificar conexões e alimentação

---

## 🤝 Contribuição

### 📝 Como Contribuir

1. **Fork** o projeto
2. **Crie uma branch** para sua feature
3. **Commit** suas mudanças
4. **Push** para a branch
5. **Abra um Pull Request**

### 🐛 Reportar Bugs

- Use as **Issues** do GitHub
- Descreva o problema detalhadamente
- Inclua logs e screenshots se possível

---

## 📄 Licença

Este projeto está sob a licença **MIT**. Veja o arquivo `LICENSE` para mais detalhes.

---

## 🙏 Agradecimentos

- **OLR-Arduino** - Projeto base inspirador
- **FastLED** - Biblioteca para controle de LEDs
- **Neo Brutalism UI** - Design system moderno
- **PlatformIO** - IDE para desenvolvimento embarcado

---

## 📞 Contato

- **GitHub:** [@felipebrito](https://github.com/felipebrito)
- **Projeto:** [autorama_4jogadores](https://github.com/felipebrito/autorama_4jogadores)

---

<div align="center">

**⭐ Se este projeto te ajudou, considere dar uma estrela!**

**🏁 Divirta-se com suas corridas de LED!**

</div>
