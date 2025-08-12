# 🏁 Open LED Race - Simulador Web

Uma aplicação web que simula fielmente o comportamento do jogo de corrida Arduino "Open LED Race", permitindo que você experimente a mesma mecânica de jogo em qualquer navegador moderno.

## 🎮 Como Funciona

Esta aplicação replica exatamente a lógica do código Arduino original:

- **2 Jogadores** competindo em uma pista virtual
- **Física realista** com aceleração, atrito e gravidade
- **Sistema de voltas** (5 voltas para vencer)
- **Efeitos visuais** simulando LEDs de uma fita LED
- **Controles responsivos** com botões e teclado

## 🚀 Como Jogar

### Controles
- **Jogador 1**: Clique no botão "ACELERAR P1" ou use a tecla **Q**
- **Jogador 2**: Clique no botão "ACELERAR P2" ou use a tecla **P**

### Mecânica
1. Clique em "Iniciar Corrida" para começar
2. Aguarde a contagem regressiva (2 segundos)
3. Use os botões de aceleração para impulsionar os carros
4. Primeiro a completar 5 voltas vence!
5. Use "Reiniciar" para começar uma nova corrida

## 🔧 Características Técnicas

### Física do Jogo
- **Aceleração**: 0.2 por clique
- **Atrito**: 0.015 (constante de fricção)
- **Gravidade**: 0.003 (afeta carros em rampas)
- **Pista**: 100 LEDs virtuais (otimizado para web)

### Efeitos Visuais
- **Carros com cauda**: Efeito de rastro como no Arduino
- **Mapa de gravidade**: Simula rampas na pista
- **Animações suaves**: Transições fluidas entre posições
- **Efeitos de vitória**: Animações especiais para o vencedor

### Áudio
- **Sons de volta**: Frequências diferentes para cada jogador
- **Música de vitória**: Sequência musical como no Arduino original
- **Web Audio API**: Implementação moderna para navegadores

## 🌐 Compatibilidade

- ✅ Navegadores modernos (Chrome, Firefox, Safari, Edge)
- ✅ Dispositivos móveis (touch controls)
- ✅ Responsivo para diferentes tamanhos de tela
- ✅ Funciona offline (sem dependências externas)

## 📁 Estrutura dos Arquivos

```
Race/
├── index.html      # Interface principal
├── styles.css      # Estilos e animações
├── script.js       # Lógica do jogo
├── arduino/
│   └── arduino_autorama_listener/
│       └── arduino_autorama_listener.ino  # Sketch Arduino receptor (Web Serial)
└── README.md       # Este arquivo
```

## 🎯 Diferenças do Arduino Original

- **Pista reduzida**: 100 LEDs em vez de 300 (para melhor visualização web)
- **Controles adaptados**: Mouse/touch + teclado em vez de botões físicos
- **Interface moderna**: Design responsivo com CSS3
- **Performance otimizada**: requestAnimationFrame para animações suaves

## 🚀 Como Executar

1. Baixe todos os arquivos para uma pasta
2. Abra `index.html` em qualquer navegador moderno
3. Clique em "Iniciar Corrida" e divirta-se!

## 🔌 Integrando com Arduino (Web Serial)

### Requisitos
- Arduino compatível com USB (UNO/Nano/Mega etc.)
- Biblioteca `Adafruit_NeoPixel`
- Fita WS2812/WS2813 no pino `6` (ajustável no sketch)

### Instalação do firmware
1. Abra o Arduino IDE
2. Carregue o sketch `arduino/arduino_autorama_listener/arduino_autorama_listener.ino`
3. Ajuste `MAX_PIXELS` e `PIN_LED` conforme seu hardware
4. Faça o upload

### Uso
1. Abra `index.html` em `http://localhost:8000`
2. Clique em `Conectar Arduino` e selecione a porta
3. Clique em `Iniciar Corrida`
4. O navegador envia frames via Web Serial (115200) com posições dos carros

Formato das mensagens enviadas (NDJSON):
```
{"type":"config","maxLed":100,"loopMax":5,"acel":0.2,"kf":0.015,"kg":0.003}
{"type":"state","dist1":123.4,"dist2":87.1,"speed1":0.55,"speed2":0.48,"loop1":2,"loop2":1,"leader":1}
```

O Arduino renderiza os carros com base em `dist%maxLed`.

## 🧭 Timing & Scoring
O painel da UI exibe:
- Volta atual, última volta e melhor volta por jogador
- Velocidade atual e média (LED/s)

## 📤 Publicação no GitHub

1. Inicialize o git e configure o remoto:
```
git init
git add .
git commit -m "feat: simulador web + timing & scoring + integração Web Serial + sketch Arduino"
git branch -M main
git remote add origin https://github.com/felipebrito/autorama_led.git
git push -u origin main
```

2. Ative GitHub Pages (opcional) para servir o `index.html` (precisa de HTTPS para Web Serial):
- Vá em Settings → Pages → Branch: `main` → `/` → Save
- Abra o link publicado (HTTPS obrigatório para o botão `Conectar Arduino`)

## 🔮 Próximas Melhorias

- [ ] Modo multiplayer online
- [ ] Mais tipos de pistas e obstáculos
- [ ] Sistema de recordes
- [ ] Efeitos visuais avançados
- [ ] Modo torneio

## 📜 Licença

Este projeto é baseado no "Open LED Race" original, que é distribuído sob a GNU General Public License v3.0.

## 🤝 Contribuições

Contribuições são bem-vindas! Sinta-se à vontade para:
- Reportar bugs
- Sugerir melhorias
- Enviar pull requests
- Compartilhar ideias para novos recursos

---

**Divirta-se correndo! 🏁🏎️**
