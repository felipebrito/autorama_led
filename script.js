class LEDRace {
    constructor() {
        // Constantes do jogo (baseadas no código Arduino)
        this.MAXLED = 100; // Reduzido para melhor visualização web
        this.ACEL = 0.2; // Aceleração
        this.kf = 0.015; // Constante de atrito
        this.kg = 0.003; // Constante de gravidade
        this.loop_max = 5; // Total de voltas
        this.tailLength = 3; // tamanho do rastro visual
        
        // Estado do jogo
        this.speed1 = 0;
        this.speed2 = 0;
        this.dist1 = 0;
        this.dist2 = 0;
        this.loop1 = 0;
        this.loop2 = 0;
        this.leader = 0;
        
        // Flags de controle
        this.flag_sw1 = 0;
        this.flag_sw2 = 0;
        this.draworder = 0;
        this.gameRunning = false;
        this.lastTimestampMs = null; // para física baseada em tempo
        this.raceStartMs = null;
        this.p1LapStartMs = null;
        this.p2LapStartMs = null;
        this.p1LapsMs = [];
        this.p2LapsMs = [];
        this.p1BestMs = null;
        this.p2BestMs = null;
        this.p1DistAtStart = 0;
        this.p2DistAtStart = 0;
        
        // Mapa de gravidade (simulando rampas)
        this.gravity_map = new Array(this.MAXLED).fill(127);
        
        // Elementos DOM
        this.track = document.getElementById('track');
        this.car1 = document.getElementById('car1');
        this.car2 = document.getElementById('car2');
        this.leaderDisplay = document.getElementById('leader');
        
        // Botões
        this.btn1 = document.getElementById('btn1');
        this.btn2 = document.getElementById('btn2');
        this.startBtn = document.getElementById('startBtn');
        this.resetBtn = document.getElementById('resetBtn');
        this.connectBtn = document.getElementById('connectBtn');
        this.serialStatus = document.getElementById('serialStatus');
        
        this.init();
    }
    
    init() {
        this.createLEDs();
        this.setupGravityMap();
        this.setupEventListeners();
        this.setupKeyboardControls();
        this.updateDisplay();

        // Configurações
        this.bindConfigPanel();
        this.initSerial();
    }
    
    createLEDs() {
        this.track.innerHTML = '';
        for (let i = 0; i < this.MAXLED; i++) {
            const led = document.createElement('div');
            led.className = 'led';
            led.id = `led-${i}`;
            this.track.appendChild(led);
        }
    }
    
    setupGravityMap() {
        // Base plana
        this.gravity_map.fill(127);

        // Rampa proporcional ao tamanho da pista
        const H = 12;
        const center = Math.max(1, Math.min(this.MAXLED - 2, Math.round(this.MAXLED * 0.33)));
        const halfWidth = Math.max(2, Math.round(this.MAXLED / 30));
        const a = Math.max(0, center - halfWidth);
        const b = center;
        const c = Math.min(this.MAXLED - 1, center + halfWidth);
        this.setRamp(H, a, b, c);

        // Aplicar efeito visual da gravidade (azul mais forte nas valas)
        for (let i = 0; i < this.MAXLED; i++) {
            const led = document.getElementById(`led-${i}`);
            const intensity = Math.max(0, Math.floor((127 - this.gravity_map[i]) / 8));
            if (led) led.style.background = `rgb(0, 0, ${intensity})`;
        }
    }
    
    setRamp(H, a, b, c) {
        // Implementação da função set_ramp do Arduino
        for (let i = 0; i < (b - a); i++) {
            this.gravity_map[a + i] = 127 - i * (H / (b - a));
        }
        this.gravity_map[b] = 127;
        for (let i = 0; i < (c - b); i++) {
            this.gravity_map[b + i + 1] = 127 + H - i * (H / (c - b));
        }
    }

    bindConfigPanel() {
        const maxLedInput = document.getElementById('cfgMaxLed');
        const loopMaxInput = document.getElementById('cfgLoopMax');
        const acelInput = document.getElementById('cfgAcel');
        const kfInput = document.getElementById('cfgKf');
        const kgInput = document.getElementById('cfgKg');
        const applyBtn = document.getElementById('applyCfgBtn');
        const defaultsBtn = document.getElementById('defaultsBtn');
        const tailInput = document.getElementById('cfgTail');

        if (!applyBtn) return; // HTML antigo sem painel

        const syncLabels = () => {
            const l1 = document.getElementById('loopMaxLabel');
            const l2 = document.getElementById('loopMaxLabel2');
            if (l1) l1.textContent = String(this.loop_max);
            if (l2) l2.textContent = String(this.loop_max);
        };

        applyBtn.addEventListener('click', () => {
            const newMaxLed = Math.max(20, Math.min(1000, Number(maxLedInput.value)));
            const newLoopMax = Math.max(1, Math.min(99, Number(loopMaxInput.value)));
            const newAcel = Math.max(0, Math.min(5, Number(acelInput.value)));
            const newKf = Math.max(0, Math.min(1, Number(kfInput.value)));
            const newKg = Math.max(0, Math.min(1, Number(kgInput.value)));
            const newTail = Math.max(0, Math.min(20, Number(tailInput.value)));

            const sizeChanged = newMaxLed !== this.MAXLED;

            this.ACEL = newAcel;
            this.kf = newKf;
            this.kg = newKg;
            this.loop_max = newLoopMax;
            this.tailLength = newTail;
            syncLabels();

            if (sizeChanged) {
                this.MAXLED = newMaxLed;
                this.gravity_map = new Array(this.MAXLED).fill(127);
                this.createLEDs();
                this.setupGravityMap();
            }

            // Reinicia estados e mantém configs
            this.resetRace();
        });

        defaultsBtn.addEventListener('click', () => {
            maxLedInput.value = 100;
            loopMaxInput.value = 5;
            acelInput.value = 0.2;
            kfInput.value = 0.015;
            kgInput.value = 0.003;
            if (tailInput) tailInput.value = 3;
        });

        // Inicializa labels
        syncLabels();
    }

    // ======= Web Serial =======
    initSerial() {
        if (!('serial' in navigator)) {
            if (this.serialStatus) this.serialStatus.textContent = 'Web Serial não suportado';
            return;
        }
        this.port = null;
        this.writer = null;
        this.reader = null;
        this.readBuffer = '';
        this.serialSendInterval = null;
        if (this.connectBtn) {
            this.connectBtn.addEventListener('click', async () => {
                try {
                    this.port = await navigator.serial.requestPort();
                    await this.port.open({ baudRate: 115200 });
                    this.writer = this.port.writable.getWriter();
                    if (this.serialStatus) this.serialStatus.textContent = 'Conectado';
                    // Enviar configuração inicial
                    await this.sendConfigToArduino();
                    // Iniciar leitura para debug/handshake
                    this.startSerialReadLoop();
                    // Iniciar envio periódico de estado
                    this.startSerialLoop();
                } catch (e) {
                    if (this.serialStatus) this.serialStatus.textContent = 'Falha ao conectar';
                }
            });
        }
    }

    async sendLine(obj) {
        if (!this.writer) return;
        const line = JSON.stringify(obj) + '\\n';
        const data = new TextEncoder().encode(line);
        try {
            await this.writer.write(data);
        } catch (err) {
            console.error('Serial write error:', err);
            if (this.serialStatus) this.serialStatus.textContent = 'Erro ao enviar';
        }
    }

    async sendConfigToArduino() {
        await this.sendLine({ type: 'config', maxLed: this.MAXLED, loopMax: this.loop_max, acel: this.ACEL, kf: this.kf, kg: this.kg, tail: this.tailLength });
    }

    startSerialLoop() {
        if (this.serialSendInterval) clearInterval(this.serialSendInterval);
        this.serialSendInterval = setInterval(() => {
            this.sendLine({
                type: 'state',
                dist1: this.dist1,
                dist2: this.dist2,
                speed1: this.speed1,
                speed2: this.speed2,
                loop1: this.loop1,
                loop2: this.loop2,
                leader: this.leader,
                running: this.gameRunning
            });
        }, 100);
    }

    startSerialReadLoop() {
        if (!this.port?.readable) return;
        this.reader = this.port.readable.getReader();
        const processChunk = (text) => {
            this.readBuffer += text;
            let idx;
            while ((idx = this.readBuffer.indexOf('\n')) >= 0) {
                const line = this.readBuffer.slice(0, idx).trim();
                this.readBuffer = this.readBuffer.slice(idx + 1);
                if (line) {
                    // Log básico e handshake
                    try {
                        const msg = JSON.parse(line);
                        if (msg.arduino === 'ready' && this.serialStatus) {
                            this.serialStatus.textContent = 'Arduino pronto';
                        }
                    } catch (_) {
                        // não JSON
                    }
                    console.debug('SERIAL RX:', line);
                }
            }
        };
        const pump = async () => {
            try {
                while (true) {
                    const { value, done } = await this.reader.read();
                    if (done) break;
                    if (value) {
                        const text = new TextDecoder().decode(value);
                        processChunk(text);
                    }
                }
            } catch (err) {
                console.warn('Serial read loop ended:', err);
            } finally {
                try { this.reader.releaseLock(); } catch (_) {}
            }
        };
        pump();
    }
    
    setupEventListeners() {
        this.btn1.addEventListener('mousedown', () => {
            if (this.gameRunning && this.flag_sw1 === 1) {
                this.flag_sw1 = 0;
                this.speed1 += this.ACEL;
            }
        });
        this.btn1.addEventListener('mouseup', () => {
            if (this.flag_sw1 === 0) {
                this.flag_sw1 = 1;
            }
        });
        this.btn1.addEventListener('mouseleave', () => {
            if (this.flag_sw1 === 0) {
                this.flag_sw1 = 1;
            }
        });
        
        this.btn2.addEventListener('mousedown', () => {
            if (this.gameRunning && this.flag_sw2 === 1) {
                this.flag_sw2 = 0;
                this.speed2 += this.ACEL;
            }
        });
        this.btn2.addEventListener('mouseup', () => {
            if (this.flag_sw2 === 0) {
                this.flag_sw2 = 1;
            }
        });
        this.btn2.addEventListener('mouseleave', () => {
            if (this.flag_sw2 === 0) {
                this.flag_sw2 = 1;
            }
        });
        
        this.startBtn.addEventListener('click', () => this.startRace());
        this.resetBtn.addEventListener('click', () => this.resetRace());
        
        // Touch events para dispositivos móveis
        this.btn1.addEventListener('touchstart', (e) => {
            e.preventDefault();
            if (this.gameRunning && this.flag_sw1 === 1) {
                this.flag_sw1 = 0;
                this.speed1 += this.ACEL;
            }
        });
        this.btn1.addEventListener('touchend', (e) => {
            e.preventDefault();
            if (this.flag_sw1 === 0) {
                this.flag_sw1 = 1;
            }
        });
        
        this.btn2.addEventListener('touchstart', (e) => {
            e.preventDefault();
            if (this.gameRunning && this.flag_sw2 === 1) {
                this.flag_sw2 = 0;
                this.speed2 += this.ACEL;
            }
        });
        this.btn2.addEventListener('touchend', (e) => {
            e.preventDefault();
            if (this.flag_sw2 === 0) {
                this.flag_sw2 = 1;
            }
        });
    }
    
    setupKeyboardControls() {
        document.addEventListener('keydown', (e) => {
            if (!this.gameRunning) return;
            
            switch(e.key.toLowerCase()) {
                case 'q':
                    if (this.flag_sw1 === 1) {
                        this.flag_sw1 = 0;
                        this.speed1 += this.ACEL;
                    }
                    break;
                case 'p':
                    if (this.flag_sw2 === 1) {
                        this.flag_sw2 = 0;
                        this.speed2 += this.ACEL;
                    }
                    break;
            }
        });
        
        document.addEventListener('keyup', (e) => {
            switch(e.key.toLowerCase()) {
                case 'q':
                    if (this.flag_sw1 === 0) {
                        this.flag_sw1 = 1;
                    }
                    break;
                case 'p':
                    if (this.flag_sw2 === 0) {
                        this.flag_sw2 = 1;
                    }
                    break;
            }
        });
    }
    

    
    startRace() {
        if (this.gameRunning) return;
        
        this.gameRunning = true;
        this.startBtn.textContent = 'Corrida em Andamento';
        this.startBtn.disabled = true;
        this.lastTimestampMs = null;
        
        // Sequência de início como no Arduino
        this.countdown();
        
        // Iniciar loop do jogo
        this.gameLoop();
    }
    
    countdown() {
        this.leaderDisplay.textContent = '🏁 Preparando...';
        
        setTimeout(() => {
            this.leaderDisplay.textContent = '🟢 Verde!';
            this.flag_sw1 = 1;
            this.flag_sw2 = 1;
            const now = performance.now();
            this.raceStartMs = now;
            this.p1LapStartMs = now;
            this.p2LapStartMs = now;
            this.p1LapsMs = [];
            this.p2LapsMs = [];
            this.p1BestMs = null;
            this.p2BestMs = null;
            this.p1DistAtStart = this.dist1;
            this.p2DistAtStart = this.dist2;
        }, 2000);
    }
    
    resetRace() {
        this.gameRunning = false;
        this.speed1 = 0;
        this.speed2 = 0;
        this.dist1 = 0;
        this.dist2 = 0;
        this.loop1 = 0;
        this.loop2 = 0;
        this.leader = 0;
        this.flag_sw1 = 0;
        this.flag_sw2 = 0;
        
        this.startBtn.textContent = 'Iniciar Corrida';
        this.startBtn.disabled = false;
        this.leaderDisplay.textContent = '🏁 Aguardando início...';
        
        this.updateDisplay();
        this.updateCarPositions();
        this.clearTrack();
        this.updateTimingUI();
    }
    
    gameLoop() {
        if (!this.gameRunning) return;

        const nowMs = performance.now();
        if (this.lastTimestampMs === null) {
            this.lastTimestampMs = nowMs;
        }
        const deltaMs = nowMs - this.lastTimestampMs;
        // Fator de normalização: 16.666ms ~ 60 FPS
        const dt = Math.max(0.001, deltaMs / 16.6667);
        this.lastTimestampMs = nowMs;

        this.updatePhysics(dt);
        this.checkLaps();
        this.checkWinner();
        this.updateDisplay();
        this.updateTimingUI();
        this.updateCarPositions();
        this.drawCars();
        
        // Continuar loop
        requestAnimationFrame(() => this.gameLoop());
    }
    
    updatePhysics(dt) {
        // Física do jogador 1 (replicando o código Arduino)
        // Removido: não devemos alterar flag_sw1 aqui.
        
        // Aplicar gravidade baseada no mapa
        const pos1 = Math.floor(this.dist1) % this.MAXLED;
        if (this.gravity_map[pos1] < 127) {
            this.speed1 -= this.kg * (127 - this.gravity_map[pos1]) * dt;
        }
        if (this.gravity_map[pos1] > 127) {
            this.speed1 += this.kg * (this.gravity_map[pos1] - 127) * dt;
        }
        
        // Aplicar atrito
        this.speed1 -= this.speed1 * this.kf * dt;
        
        // Física do jogador 2
        // Removido: não devemos alterar flag_sw2 aqui.
        
        const pos2 = Math.floor(this.dist2) % this.MAXLED;
        if (this.gravity_map[pos2] < 127) {
            this.speed2 -= this.kg * (127 - this.gravity_map[pos2]) * dt;
        }
        if (this.gravity_map[pos2] > 127) {
            this.speed2 += this.kg * (this.gravity_map[pos2] - 127) * dt;
        }
        
        this.speed2 -= this.speed2 * this.kf * dt;
        
        // Atualizar posições
        this.dist1 += this.speed1 * dt;
        this.dist2 += this.speed2 * dt;
        
        // Determinar líder
        if (this.dist1 > this.dist2) {
            this.leader = 1;
        } else if (this.dist2 > this.dist1) {
            this.leader = 2;
        }
        
        // Alternar ordem de desenho
        if (Math.random() < 0.1) {
            this.draworder = this.draworder === 0 ? 1 : 0;
        }
    }
    
    checkLaps() {
        // Calcular voltas com base na distância, garantindo que nunca diminua
        const computedLoop1 = Math.floor(Math.max(0, this.dist1 - this.p1DistAtStart) / this.MAXLED);
        const computedLoop2 = Math.floor(Math.max(0, this.dist2 - this.p2DistAtStart) / this.MAXLED);

        if (computedLoop1 > this.loop1) {
            const increments = computedLoop1 - this.loop1;
            this.loop1 = computedLoop1;
            // Reproduzir um beep por volta ganho (limite 3 para não acumular muitos sons)
            const plays = Math.min(3, increments);
            for (let i = 0; i < plays; i++) {
                setTimeout(() => this.playLapSound(600), i * 80);
            }
            // Registrar tempos de volta p1
            const now = performance.now();
            const lapMs = now - (this.p1LapStartMs ?? now);
            this.p1LapStartMs = now;
            this.p1LapsMs.push(lapMs);
            this.p1BestMs = this.p1BestMs === null ? lapMs : Math.min(this.p1BestMs, lapMs);
        }

        if (computedLoop2 > this.loop2) {
            const increments = computedLoop2 - this.loop2;
            this.loop2 = computedLoop2;
            const plays = Math.min(3, increments);
            for (let i = 0; i < plays; i++) {
                setTimeout(() => this.playLapSound(700), i * 80);
            }
            // Registrar tempos de volta p2
            const now2 = performance.now();
            const lapMs2 = now2 - (this.p2LapStartMs ?? now2);
            this.p2LapStartMs = now2;
            this.p2LapsMs.push(lapMs2);
            this.p2BestMs = this.p2BestMs === null ? lapMs2 : Math.min(this.p2BestMs, lapMs2);
        }
    }

    msToLapString(ms) {
        if (!ms && ms !== 0) return '—';
        const totalMs = Math.max(0, Math.round(ms));
        const minutes = Math.floor(totalMs / 60000);
        const seconds = Math.floor((totalMs % 60000) / 1000);
        const millis = totalMs % 1000;
        const mm = minutes > 0 ? `${minutes}:` : '';
        const ss = `${seconds}`.padStart(2, '0');
        const mmm = `${millis}`.padStart(3, '0');
        return `${mm}${ss}.${mmm}`;
    }

    updateTimingUI() {
        // Velocidade atual (LED/s) e média
        const p1SpeedNow = this.speed1.toFixed(2);
        const p2SpeedNow = this.speed2.toFixed(2);
        const raceElapsed = this.raceStartMs ? (performance.now() - this.raceStartMs) / 1000 : 0;
        const p1Avg = raceElapsed > 0 ? ((this.dist1 - this.p1DistAtStart) / raceElapsed).toFixed(2) : '0.00';
        const p2Avg = raceElapsed > 0 ? ((this.dist2 - this.p2DistAtStart) / raceElapsed).toFixed(2) : '0.00';

        const setText = (id, value) => { const el = document.getElementById(id); if (el) el.textContent = value; };
        setText('p1SpeedNow', p1SpeedNow);
        setText('p2SpeedNow', p2SpeedNow);
        setText('p1SpeedAvg', p1Avg);
        setText('p2SpeedAvg', p2Avg);

        // Volta atual (cronômetro de volta em andamento)
        if (this.p1LapStartMs) setText('p1CurrentLap', this.msToLapString(performance.now() - this.p1LapStartMs));
        if (this.p2LapStartMs) setText('p2CurrentLap', this.msToLapString(performance.now() - this.p2LapStartMs));

        // Última e melhor volta
        setText('p1LastLap', this.msToLapString(this.p1LapsMs[this.p1LapsMs.length - 1]));
        setText('p2LastLap', this.msToLapString(this.p2LapsMs[this.p2LapsMs.length - 1]));
        setText('p1BestLap', this.msToLapString(this.p1BestMs));
        setText('p2BestLap', this.msToLapString(this.p2BestMs));

        // Listas de voltas
        const renderLaps = (containerId, laps) => {
            const el = document.getElementById(containerId);
            if (!el) return;
            el.innerHTML = laps.map((ms, i) => `<div>Lap ${i + 1}: ${this.msToLapString(ms)}</div>`).join('');
        };
        renderLaps('p1Laps', this.p1LapsMs);
        renderLaps('p2Laps', this.p2LapsMs);
    }
    
    checkWinner() {
        if (this.loop1 >= this.loop_max) {
            this.declareWinner(1);
        } else if (this.loop2 >= this.loop_max) {
            this.declareWinner(2);
        }
    }
    
    declareWinner(player) {
        this.gameRunning = false;
        this.startBtn.textContent = 'Iniciar Corrida';
        this.startBtn.disabled = false;
        
        if (player === 1) {
            this.leaderDisplay.textContent = '🏆 Jogador 1 Venceu! 🏆';
            this.track.classList.add('winner');
            this.car1.classList.add('winner');
        } else {
            this.leaderDisplay.textContent = '🏆 Jogador 2 Venceu! 🏆';
            this.track.classList.add('winner');
            this.car2.classList.add('winner');
        }
        
        this.playWinnerSound();
        
        // Reiniciar após 3 segundos
        setTimeout(() => {
            this.resetRace();
            this.track.classList.remove('winner');
            this.car1.classList.remove('winner');
            this.car2.classList.remove('winner');
        }, 3000);
    }
    
    updateDisplay() {
        document.getElementById('speed1').textContent = this.speed1.toFixed(2);
        document.getElementById('speed2').textContent = this.speed2.toFixed(2);
        document.getElementById('dist1').textContent = Math.floor(this.dist1);
        document.getElementById('dist2').textContent = Math.floor(this.dist2);
        document.getElementById('loop1').textContent = this.loop1;
        document.getElementById('loop2').textContent = this.loop2;
        
        // Atualizar display do líder
        if (this.gameRunning) {
            if (this.leader === 1) {
                this.leaderDisplay.textContent = '🔴 Jogador 1 na frente!';
            } else if (this.leader === 2) {
                this.leaderDisplay.textContent = '🟢 Jogador 2 na frente!';
            } else {
                this.leaderDisplay.textContent = '🏁 Corrida em andamento...';
            }
        }
    }
    
    updateCarPositions() {
        // Posicionar carros alinhados ao centro do LED real
        const container = this.track.parentElement; // .track-container
        if (!container) return;

        const idx1 = Math.floor(((this.dist1 % this.MAXLED) + this.MAXLED) % this.MAXLED);
        const idx2 = Math.floor(((this.dist2 % this.MAXLED) + this.MAXLED) % this.MAXLED);
        const led1 = document.getElementById(`led-${idx1}`);
        const led2 = document.getElementById(`led-${idx2}`);
        if (!led1 || !led2) return;

        const containerRect = container.getBoundingClientRect();
        const led1Rect = led1.getBoundingClientRect();
        const led2Rect = led2.getBoundingClientRect();
        const carWidth = this.car1.offsetWidth || 20;

        const left1 = led1Rect.left - containerRect.left + led1Rect.width / 2 - carWidth / 2;
        const left2 = led2Rect.left - containerRect.left + led2Rect.width / 2 - carWidth / 2;

        this.car1.style.left = `${left1}px`;
        this.car2.style.left = `${left2}px`;
    }
    
    drawCars() {
        // Limpar track
        this.clearTrack();
        
        // Desenhar carros na ordem correta
        if (this.draworder === 0) {
            this.drawCar1();
            this.drawCar2();
        } else {
            this.drawCar2();
            this.drawCar1();
        }
    }
    
    drawCar1() {
        // Desenhar carro 1 com efeito de "cauda" como no Arduino
        const trail = Math.max(0, this.tailLength);
        for (let i = 0; i <= trail; i++) {
            const pos = (Math.floor(this.dist1) + i) % this.MAXLED;
            const led = document.getElementById(`led-${pos}`);
            if (led) {
                const intensity = Math.max(0, 255 - i * 20);
                led.style.background = `rgb(0, ${intensity}, 0)`;
                led.classList.add('active');
            }
        }
    }
    
    drawCar2() {
        // Desenhar carro 2 com efeito de "cauda"
        const trail = Math.max(0, this.tailLength);
        for (let i = 0; i <= trail; i++) {
            const pos = (Math.floor(this.dist2) + i) % this.MAXLED;
            const led = document.getElementById(`led-${pos}`);
            if (led) {
                const intensity = Math.max(0, 255 - i * 20);
                led.style.background = `rgb(${intensity}, 0, 0)`;
                led.classList.add('active');
            }
        }
    }
    
    clearTrack() {
        // Limpar todos os LEDs
        for (let i = 0; i < this.MAXLED; i++) {
            const led = document.getElementById(`led-${i}`);
            if (led) {
                led.classList.remove('active');
                // Manter efeito de gravidade
                const intensity = Math.floor((127 - this.gravity_map[i]) / 8);
                led.style.background = `rgb(0, 0, ${Math.max(0, intensity)})`;
            }
        }
    }
    
    playLapSound(frequency) {
        // Simular som de volta usando Web Audio API
        try {
            const audioContext = new (window.AudioContext || window.webkitAudioContext)();
            const oscillator = audioContext.createOscillator();
            const gainNode = audioContext.createGain();
            
            oscillator.connect(gainNode);
            gainNode.connect(audioContext.destination);
            
            oscillator.frequency.setValueAtTime(frequency, audioContext.currentTime);
            oscillator.type = 'sine';
            
            gainNode.gain.setValueAtTime(0.1, audioContext.currentTime);
            gainNode.gain.exponentialRampToValueAtTime(0.01, audioContext.currentTime + 0.2);
            
            oscillator.start(audioContext.currentTime);
            oscillator.stop(audioContext.currentTime + 0.2);
        } catch (e) {
            // Fallback silencioso se Web Audio API não estiver disponível
        }
    }
    
    playWinnerSound() {
        // Música de vitória como no Arduino
        const winMusic = [2093, 2637, 3136, 2093, 2637, 3136, 2093, 2637, 3136];
        
        winMusic.forEach((freq, index) => {
            setTimeout(() => {
                this.playLapSound(freq);
            }, index * 230);
        });
    }
}

// Inicializar o jogo quando a página carregar
document.addEventListener('DOMContentLoaded', () => {
    new LEDRace();
});
