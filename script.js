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
        this.flag_sw1 = 1;
        this.flag_sw2 = 1;
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
        this.serialInfo = document.getElementById('serialInfo');
        this.listBtn = document.getElementById('listBtn');
        this.disconnectBtn = document.getElementById('disconnectBtn');
        this.testEffectBtn = document.getElementById('testEffectBtn');
        
        // NOVOS BOTÕES DE TESTE DIRETO
        this.testJogador1Btn = document.getElementById('testJogador1Btn');
        this.testJogador2Btn = document.getElementById('testJogador2Btn');
        this.testSimularBtn = document.getElementById('testSimularBtn');
        this.testDemoBtn = document.getElementById('testDemoBtn');
        this.testLimparBtn = document.getElementById('testLimparBtn');
        
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
        // this.setupTestButtons(); // Inicializa os botões de teste - Moved to openSelectedPort
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
        this.serialInfo = document.getElementById('serialInfo');
        this.listBtn = document.getElementById('listBtn');
        this.disconnectBtn = document.getElementById('disconnectBtn');
        if (this.connectBtn) {
            this.connectBtn.addEventListener('click', async () => {
                console.log('[UI] connectBtn clicked');
                try {
                    // fecha conexão anterior, se houver
                try { this.closeSerial?.(); } catch (e) { console.warn('[SERIAL] close before connect ignored:', e?.message||e); }
                    this.port = await navigator.serial.requestPort();
                    const info = this.port.getInfo ? this.port.getInfo() : {};
                    console.log('[SERIAL] requestPort resolved:', info);
                    await this.openSelectedPort(this.port);
                } catch (e) {
                    console.error('[ERROR] Serial connect error:', e?.name, e?.message || e);
                    if (this.serialStatus) this.serialStatus.textContent = 'Falha ao conectar';
                    if (this.serialInfo) this.serialInfo.textContent = 'Possível porta ocupada. Feche o Serial Monitor do Arduino IDE ou outro app que use a mesma porta e tente novamente.';
                }
            });
        }

        if (this.listBtn) {
            this.listBtn.addEventListener('click', async () => {
                console.log('[UI] listBtn clicked');
                try {
                    const ports = await navigator.serial.getPorts();
                    const summary = ports.map((p, i) => {
                        try { return JSON.stringify(p.getInfo()); } catch { return 'port ' + i; }
                    }).join('\n');
                    console.log('[SERIAL] Authorized ports:', ports);
                    if (this.serialInfo) this.serialInfo.textContent = 'Portas autorizadas:\n' + (summary || 'nenhuma');
                } catch (e) {
                    console.error('[ERROR] list ports:', e);
                    if (this.serialInfo) this.serialInfo.textContent = 'Erro ao listar portas: ' + (e?.message || e);
                }
            });
        }

        if (this.disconnectBtn) {
            this.disconnectBtn.addEventListener('click', async () => {
                console.log('[UI] disconnectBtn clicked');
                this.closeSerial();
                if (this.serialStatus) this.serialStatus.textContent = 'Desconectado';
                if (this.serialInfo) this.serialInfo.textContent = 'Conexão encerrada pelo usuário';
            });
        }

        if (this.testEffectBtn) {
            this.testEffectBtn.addEventListener('click', async () => {
                console.log('[UI] testEffectBtn clicked');
                
                if (!this.writer) {
                    console.warn('[UI] Arduino não conectado!');
                    alert('Conecte o Arduino primeiro!');
                    return;
                }
                
                try {
                    // Envia um ping e um efeito de teste
                    console.log('[UI] Enviando ping...');
                    await this.sendLine({ type: 'ping' });
                    await this.sleep(200);
                    
                    console.log('[UI] Enviando effect...');
                    await this.sendLine({ type: 'effect', name: 'flash' });
                    await this.sleep(200);
                    
                    // Se corrida não estiver ativa, envia um frame de state com running=1 e dist aleatória
                    if (!this.gameRunning) {
                        console.log('[UI] Enviando state de teste...');
                        const d = Math.floor(Math.random() * this.MAXLED);
                        await this.sendLine({ 
                            type: 'state', 
                            dist1: d, 
                            dist2: (d + Math.floor(this.MAXLED/2)) % this.MAXLED, 
                            speed1: 0, 
                            speed2: 0, 
                            loop1: 0, 
                            loop2: 0, 
                            leader: 0, 
                            running: 1 
                        });
                    }
                    
                    console.log('[UI] Teste concluído!');
                } catch (error) {
                    console.error('[UI] Erro no teste:', error);
                    alert('Erro ao testar LEDs: ' + error.message);
                }
            });
        }

        // Eventos de plug/unplug → tentativa de autoconectar a uma porta já autorizada
        navigator.serial.addEventListener('connect', async () => {
            console.log('[SERIAL] device connected');
            if (!this.port) {
                try {
                    const ports = await navigator.serial.getPorts();
                    if (ports && ports.length) {
                        await this.openSelectedPort(ports[0]);
                    }
                } catch (e) {
                    console.warn('[SERIAL] auto-connect failed:', e?.message || e);
                }
            }
        });
        navigator.serial.addEventListener('disconnect', () => {
            console.warn('[SERIAL] device disconnected');
            this.closeSerial();
            if (this.serialStatus) this.serialStatus.textContent = 'Desconectado';
            if (this.serialInfo) this.serialInfo.textContent = 'Dispositivo desconectado';
        });
    }

    async openSelectedPort(port) {
        if (!port) throw new Error('No port provided');
        // Se já estiver aberta, segue direto
        if (!port.readable && !port.writable) {
            await port.open({ baudRate: 115200 });
            console.log('[SERIAL] port opened @115200');
        } else {
            console.warn('[SERIAL] port already open, reusing instance');
        }
        // DTR/RTS toggle para reset
        try {
            await port.setSignals({ dataTerminalReady: false, requestToSend: false });
            await this.sleep(150);
            await port.setSignals({ dataTerminalReady: true, requestToSend: true });
            await this.sleep(150);
            console.log('[SERIAL] DTR/RTS toggled');
        } catch (e) {
            console.warn('[SERIAL] setSignals not supported or failed:', e?.message || e);
        }
        this.port = port;
        this.writer = this.port.writable.getWriter();
        if (this.serialStatus) this.serialStatus.textContent = 'Conectado';
        
        // INICIALIZAR BOTÕES DE TESTE APENAS DEPOIS DA CONEXÃO
        this.setupTestButtons();
        
        await this.sendConfigToArduino();
        console.log('[SERIAL] config sent');
        this.startSerialReadLoop();
        this.startSerialLoop();
        console.log('[SERIAL] state loop started');
    }

    async sendLine(obj) {
        if (!this.writer) return;
        const line = JSON.stringify(obj) + '\n';
        const data = new TextEncoder().encode(line);
        try {
            await this.writer.write(data);
            // Log de debug para todas as mensagens enviadas
            if (obj.type === 'state' && this.gameRunning) {
                console.log('[SERIAL TX] Estado enviado:', JSON.stringify(obj));
            } else if (obj.type === 'config') {
                console.log('[SERIAL TX] Config enviada:', JSON.stringify(obj));
            } else if (obj.type === 'ping' || obj.type === 'effect') {
                console.log('[SERIAL TX] Comando de teste enviado:', JSON.stringify(obj));
            }
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
            const stateData = {
                type: 'state',
                dist1: this.dist1,
                dist2: this.dist2,
                speed1: this.speed1,
                speed2: this.speed2,
                loop1: this.loop1,
                loop2: this.loop2,
                leader: this.leader,
                // Envie running=true sempre que a corrida estiver ativa; caso contrário true se quiser parar demo ao parar
                running: this.gameRunning ? 1 : 0
            };
            
            // Log detalhado do estado sendo enviado
            if (this.gameRunning) {
                console.log('[SERIAL DEBUG] Enviando estado:', {
                    dist1: this.dist1.toFixed(2),
                    dist2: this.dist2.toFixed(2),
                    speed1: this.speed1.toFixed(2),
                    speed2: this.speed2.toFixed(2),
                    running: this.gameRunning ? 1 : 0,
                    timestamp: Date.now()
                });
            }
            
            this.sendLine(stateData);
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

    closeSerial() {
        try { if (this.serialSendInterval) clearInterval(this.serialSendInterval); } catch (_) {}
        try { if (this.reader) this.reader.cancel(); } catch (_) {}
        try { if (this.reader) this.reader.releaseLock(); } catch (_) {}
        try { if (this.writer) this.writer.releaseLock(); } catch (_) {}
        try { if (this.port) this.port.close(); } catch (e) { console.warn('[SERIAL] close() ignored:', e?.message||e); }
        this.port = null;
        this.reader = null;
        this.writer = null;
    }

    sleep(ms) { return new Promise(res => setTimeout(res, ms)); }
    
    setupEventListeners() {
        this.btn1.addEventListener('mousedown', () => {
            if (this.gameRunning && this.flag_sw1 === 1) {
                this.flag_sw1 = 0;
                this.speed1 += this.ACEL;
            }
        });
        this.btn1.addEventListener('mouseup', () => {
            this.flag_sw1 = 1;
        });
        this.btn1.addEventListener('mouseleave', () => {
            this.flag_sw1 = 1;
        });
        
        this.btn2.addEventListener('mousedown', () => {
            if (this.gameRunning && this.flag_sw2 === 1) {
                this.flag_sw2 = 0;
                this.speed2 += this.ACEL;
            }
        });
        this.btn2.addEventListener('mouseup', () => {
            this.flag_sw2 = 1;
        });
        this.btn2.addEventListener('mouseleave', () => {
            this.flag_sw2 = 1;
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
            this.flag_sw1 = 1;
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
            this.flag_sw2 = 1;
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
                    this.flag_sw1 = 1;
                    break;
                case 'p':
                    this.flag_sw2 = 1;
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
        
        // Log para debug serial
        console.log('[GAME DEBUG] Corrida iniciada, gameRunning =', this.gameRunning);
        
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
        this.flag_sw1 = 1;
        this.flag_sw2 = 1;
        
        // Log para debug serial
        console.log('[GAME DEBUG] Corrida parada, gameRunning =', this.gameRunning);
        
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
        
        // Log para debug serial
        console.log('[GAME DEBUG] Corrida terminada, gameRunning =', this.gameRunning, 'Vencedor:', player);
        
        this.leaderDisplay.textContent = `🏆 Jogador ${player} venceu!`;
        
        // Parar o loop do jogo
        this.lastTimestampMs = null;
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

    // NOVAS FUNÇÕES DE TESTE DIRETO
    async sendSimpleCommand(command) {
        if (!this.writer) {
            console.warn('[UI] Arduino não conectado!');
            alert('Conecte o Arduino primeiro!');
            return;
        }
        
        try {
            console.log(`[UI] Enviando comando direto: ${command}`);
            const data = new TextEncoder().encode(command + '\n');
            await this.writer.write(data);
            console.log(`[UI] Comando ${command} enviado com sucesso!`);
        } catch (error) {
            console.error(`[UI] Erro ao enviar comando ${command}:`, error);
            alert(`Erro ao enviar comando ${command}: ` + error.message);
        }
    }

    setupTestButtons() {
        // Botão Testar Jogador 1
        if (this.testJogador1Btn) {
            this.testJogador1Btn.addEventListener('click', async () => {
                console.log('[UI] testJogador1Btn clicked');
                await this.sendSimpleCommand('jogador1');
            });
        }

        // Botão Testar Jogador 2
        if (this.testJogador2Btn) {
            this.testJogador2Btn.addEventListener('click', async () => {
                console.log('[UI] testJogador2Btn clicked');
                await this.sendSimpleCommand('jogador2');
            });
        }

        // Botão Simular
        if (this.testSimularBtn) {
            this.testSimularBtn.addEventListener('click', async () => {
                console.log('[UI] testSimularBtn clicked');
                await this.sendSimpleCommand('simular');
            });
        }

        // Botão Demo
        if (this.testDemoBtn) {
            this.testDemoBtn.addEventListener('click', async () => {
                console.log('[UI] testDemoBtn clicked');
                await this.sendSimpleCommand('demo');
            });
        }

        // Botão Limpar
        if (this.testLimparBtn) {
            this.testLimparBtn.addEventListener('click', async () => {
                console.log('[UI] testLimparBtn clicked');
                await this.sendSimpleCommand('limpar');
            });
        }
    }
}

// Inicializar o jogo quando a página carregar
document.addEventListener('DOMContentLoaded', () => {
    new LEDRace();
});