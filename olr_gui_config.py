#!/usr/bin/env python3
"""
OLR GUI Config - Interface gráfica com sliders para configuração em tempo real
Sliders para aceleração, velocidade máxima, inicial e controles do jogo
"""

import tkinter as tk
from tkinter import ttk, messagebox
import serial
import threading
import time
import sys
from colorama import init, Fore, Back, Style

init(autoreset=True)

class OLRGUIConfig:
    def __init__(self, port="/dev/cu.usbserial-110", baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        self.running = True
        
        # Valores atuais
        self.acceleration_rate = 0.3
        self.max_speed = 8.0
        self.initial_speed = 0.1
        
        # Criar interface
        self.root = tk.Tk()
        self.setup_gui()
        
    def connect(self):
        """Conectar à ESP via serial"""
        try:
            self.ser = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                timeout=1
            )
            self.status_label.config(text="✅ Conectado", fg="green")
            return True
        except Exception as e:
            self.status_label.config(text=f"❌ Erro: {e}", fg="red")
            return False
    
    def send_command(self, command):
        """Enviar comando via serial"""
        if not self.ser:
            messagebox.showerror("Erro", "ESP não conectada!")
            return False
        
        try:
            # Log do comando enviado
            self.log_message(f"📤 Enviando: {command}")
            
            # Enviar comando
            self.ser.write(f"{command}\n".encode())
            
            # Pequena pausa para processar
            time.sleep(0.1)
            
            # Ler resposta se disponível
            if self.ser.in_waiting:
                response = self.ser.readline().decode().strip()
                if response:
                    self.log_message(f"📡 ESP: {response}")
            
            return True
        except Exception as e:
            error_msg = f"Erro ao enviar comando: {e}"
            self.log_message(f"❌ {error_msg}")
            print(error_msg)
            return False
    
    def setup_gui(self):
        """Configurar interface gráfica"""
        self.root.title("🏁 OLR ESP32 - Configurador Visual")
        self.root.geometry("800x700")
        self.root.configure(bg='#2b2b2b')
        
        # Estilo
        style = ttk.Style()
        style.theme_use('clam')
        style.configure('TFrame', background='#2b2b2b')
        style.configure('TLabel', background='#2b2b2b', foreground='white')
        style.configure('TButton', background='#4a4a4a', foreground='white')
        
        # Frame principal
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=20)
        
        # Título
        title_label = tk.Label(main_frame, text="🏁 OLR ESP32 - CONFIGURADOR VISUAL", 
                              font=("Arial", 18, "bold"), fg="#00ff00", bg="#2b2b2b")
        title_label.pack(pady=(0, 20))
        
        # Status da conexão
        status_frame = ttk.Frame(main_frame)
        status_frame.pack(fill=tk.X, pady=(0, 20))
        
        self.status_label = tk.Label(status_frame, text="🔌 Desconectado", 
                                    font=("Arial", 12), fg="red", bg="#2b2b2b")
        self.status_label.pack(side=tk.LEFT)
        
        connect_btn = tk.Button(status_frame, text="🔌 Conectar", command=self.connect,
                               bg="#4a4a4a", fg="white", font=("Arial", 10))
        connect_btn.pack(side=tk.RIGHT)
        
        # Frame de configurações
        config_frame = ttk.Frame(main_frame)
        config_frame.pack(fill=tk.BOTH, expand=True)
        
        # === CONFIGURAÇÕES DE VELOCIDADE ===
        speed_frame = tk.LabelFrame(config_frame, text="⚡ CONFIGURAÇÕES DE VELOCIDADE", 
                                   font=("Arial", 14, "bold"), fg="#00ff00", bg="#2b2b2b")
        speed_frame.pack(fill=tk.X, pady=(0, 20))
        
        # Aceleração
        accel_frame = ttk.Frame(speed_frame)
        accel_frame.pack(fill=tk.X, padx=20, pady=10)
        
        tk.Label(accel_frame, text="🚀 Aceleração:", font=("Arial", 12), 
                fg="white", bg="#2b2b2b").pack(side=tk.LEFT)
        
        self.accel_slider = tk.Scale(accel_frame, from_=1, to=50, orient=tk.HORIZONTAL,
                                    length=300, bg="#2b2b2b", fg="white", 
                                    highlightbackground="#2b2b2b", troughcolor="#4a4a4a")
        self.accel_slider.set(5)
        self.accel_slider.pack(side=tk.LEFT, padx=(20, 0))
        
        self.accel_value = tk.Label(accel_frame, text="0.05", font=("Arial", 12), 
                                   fg="#00ff00", bg="#2b2b2b", width=8)
        self.accel_value.pack(side=tk.LEFT, padx=(20, 0))
        
        # Velocidade máxima
        max_frame = ttk.Frame(speed_frame)
        max_frame.pack(fill=tk.X, padx=20, pady=10)
        
        tk.Label(max_frame, text="🚀 Velocidade Máx:", font=("Arial", 12), 
                fg="white", bg="#2b2b2b").pack(side=tk.LEFT)
        
        self.max_slider = tk.Scale(max_frame, from_=5, to=100, orient=tk.HORIZONTAL,
                                  length=300, bg="#2b2b2b", fg="white", 
                                  highlightbackground="#2b2b2b", troughcolor="#4a4a4a")
        self.max_slider.set(20)
        self.max_slider.pack(side=tk.LEFT, padx=(20, 0))
        
        self.max_value = tk.Label(max_frame, text="2.0", font=("Arial", 12), 
                                 fg="#00ff00", bg="#2b2b2b", width=8)
        self.max_value.pack(side=tk.LEFT, padx=(20, 0))
        
        # Velocidade inicial
        init_frame = ttk.Frame(speed_frame)
        init_frame.pack(fill=tk.X, padx=20, pady=10)
        
        tk.Label(init_frame, text="🎯 Velocidade Inicial:", font=("Arial", 12), 
                fg="white", bg="#2b2b2b").pack(side=tk.LEFT)
        
        self.init_slider = tk.Scale(init_frame, from_=1, to=50, orient=tk.HORIZONTAL,
                                   length=300, bg="#2b2b2b", fg="white", 
                                   highlightbackground="#2b2b2b", troughcolor="#4a4a4a")
        self.init_slider.set(1)
        self.init_slider.pack(side=tk.LEFT, padx=(20, 0))
        
        self.init_value = tk.Label(init_frame, text="0.01", font=("Arial", 12), 
                                  fg="#00ff00", bg="#2b2b2b", width=8)
        self.init_value.pack(side=tk.LEFT, padx=(20, 0))
        
        # Botão aplicar configurações
        apply_btn = tk.Button(speed_frame, text="⚡ APLICAR CONFIGURAÇÕES", 
                             command=self.apply_config, bg="#00aa00", fg="white", 
                             font=("Arial", 12, "bold"))
        apply_btn.pack(pady=20)
        
        # === CONTROLES DO JOGO ===
        game_frame = tk.LabelFrame(config_frame, text="🎮 CONTROLES DO JOGO", 
                                  font=("Arial", 14, "bold"), fg="#00ff00", bg="#2b2b2b")
        game_frame.pack(fill=tk.X, pady=(0, 20))
        
        # Botões principais
        main_controls = ttk.Frame(game_frame)
        main_controls.pack(pady=20)
        
        tk.Button(main_controls, text="🏁 GO", command=lambda: self.send_command("g"),
                  bg="#00aa00", fg="white", font=("Arial", 12, "bold"), width=10).pack(side=tk.LEFT, padx=5)
        
        tk.Button(main_controls, text="🔄 RESET", command=lambda: self.send_command("r"),
                  bg="#aa0000", fg="white", font=("Arial", 12, "bold"), width=10).pack(side=tk.LEFT, padx=5)
        
        tk.Button(main_controls, text="🧪 TESTE", command=lambda: self.send_command("t"),
                  bg="#aa00aa", fg="white", font=("Arial", 12, "bold"), width=10).pack(side=tk.LEFT, padx=5)
        
        tk.Button(main_controls, text="🏔️ RAMPA", command=lambda: self.send_command("1"),
                  bg="#00aaaa", fg="white", font=("Arial", 12, "bold"), width=10).pack(side=tk.LEFT, padx=5)
        
        # Controles dos carros
        cars_frame = ttk.Frame(game_frame)
        cars_frame.pack(pady=20)
        
        tk.Label(cars_frame, text="🚗 CONTROLES DOS CARROS:", font=("Arial", 12, "bold"), 
                fg="white", bg="#2b2b2b").pack()
        
        # Acelerar
        accel_cars = tk.Frame(cars_frame, bg="#2b2b2b")
        accel_cars.pack(pady=10)
        
        tk.Label(accel_cars, text="⬆️ Acelerar:", fg="white", bg="#2b2b2b", font=("Arial", 10, "bold")).pack(side=tk.LEFT)
        
        # Botões de aceleração com cores mais visíveis
        car1_accel = tk.Button(accel_cars, text="Carro 1", command=lambda: self.send_command("a"),
                               bg="#ff00ff", fg="white", width=10, font=("Arial", 10, "bold"),
                               relief=tk.RAISED, bd=2)
        car1_accel.pack(side=tk.LEFT, padx=5)
        
        car2_accel = tk.Button(accel_cars, text="Carro 2", command=lambda: self.send_command("2"),
                               bg="#00ff00", fg="white", width=10, font=("Arial", 10, "bold"),
                               relief=tk.RAISED, bd=2)
        car2_accel.pack(side=tk.LEFT, padx=5)
        
        car3_accel = tk.Button(accel_cars, text="Carro 3", command=lambda: self.send_command("d"),
                               bg="#0000ff", fg="white", width=10, font=("Arial", 10, "bold"),
                               relief=tk.RAISED, bd=2)
        car3_accel.pack(side=tk.LEFT, padx=5)
        
        car4_accel = tk.Button(accel_cars, text="Carro 4", command=lambda: self.send_command("f"),
                               bg="#ffff00", fg="black", width=10, font=("Arial", 10, "bold"),
                               relief=tk.RAISED, bd=2)
        car4_accel.pack(side=tk.LEFT, padx=5)
        
        # Frear
        brake_cars = tk.Frame(cars_frame, bg="#2b2b2b")
        brake_cars.pack(pady=10)
        
        tk.Label(brake_cars, text="⬇️ Frear:", fg="white", bg="#2b2b2b", font=("Arial", 10, "bold")).pack(side=tk.LEFT)
        
        # Botões de freio com cores mais visíveis
        car1_brake = tk.Button(brake_cars, text="Carro 1", command=lambda: self.send_command("l"),
                               bg="#ff00ff", fg="white", width=10, font=("Arial", 10, "bold"),
                               relief=tk.RAISED, bd=2)
        car1_brake.pack(side=tk.LEFT, padx=5)
        
        car2_brake = tk.Button(brake_cars, text="Carro 2", command=lambda: self.send_command("z"),
                               bg="#00ff00", fg="white", width=10, font=("Arial", 10, "bold"),
                               relief=tk.RAISED, bd=2)
        car2_brake.pack(side=tk.LEFT, padx=5)
        
        car3_brake = tk.Button(brake_cars, text="Carro 3", command=lambda: self.send_command("c"),
                               bg="#0000ff", fg="white", width=10, font=("Arial", 10, "bold"),
                               relief=tk.RAISED, bd=2)
        car3_brake.pack(side=tk.LEFT, padx=5)
        
        car4_brake = tk.Button(brake_cars, text="Carro 4", command=lambda: self.send_command("b"),
                               bg="#ffff00", fg="black", width=10, font=("Arial", 10, "bold"),
                               relief=tk.RAISED, bd=2)
        car4_brake.pack(side=tk.LEFT, padx=5)
        
        # === SCORE E TEMPOS ===
        score_frame = tk.LabelFrame(config_frame, text="🏆 SCORE E TEMPOS", 
                                   font=("Arial", 14, "bold"), fg="#00ff00", bg="#2b2b2b")
        score_frame.pack(fill=tk.X, pady=(0, 20))
        
        # Frame para os scores dos carros
        cars_score_frame = ttk.Frame(score_frame)
        cars_score_frame.pack(fill=tk.X, padx=20, pady=20)
        
        # Título dos scores
        tk.Label(cars_score_frame, text="🚗 TEMPOS POR CARRO:", font=("Arial", 12, "bold"), 
                fg="white", bg="#2b2b2b").pack()
        
        # Grid de scores (4 carros x 3 colunas: volta, tempo, melhor)
        score_grid = ttk.Frame(cars_score_frame)
        score_grid.pack(pady=10)
        
        # Cabeçalhos
        headers = ["Carro", "Volta Atual", "Tempo Volta", "Melhor Tempo"]
        for i, header in enumerate(headers):
            tk.Label(score_grid, text=header, font=("Arial", 10, "bold"), 
                    fg="#00ff00", bg="#2b2b2b", width=12).grid(row=0, column=i, padx=5, pady=5)
        
        # Scores dos carros (serão atualizados dinamicamente)
        self.car_scores = {}
        car_colors = ["#ff00ff", "#00ff00", "#0000ff", "#ffff00"]  # Carro 1 = magenta
        car_names = ["Carro 1", "Carro 2", "Carro 3", "Carro 4"]
        
        for i in range(4):
            # Nome do carro
            tk.Label(score_grid, text=car_names[i], font=("Arial", 10), 
                    fg=car_colors[i], bg="#2b2b2b", width=12).grid(row=i+1, column=0, padx=5, pady=2)
            
            # Volta atual
            lap_label = tk.Label(score_grid, text="0", font=("Arial", 10), 
                                fg="white", bg="#2b2b2b", width=12)
            lap_label.grid(row=i+1, column=1, padx=5, pady=2)
            
            # Tempo da volta atual
            time_label = tk.Label(score_grid, text="--:--", font=("Arial", 10), 
                                 fg="white", bg="#2b2b2b", width=12)
            time_label.grid(row=i+1, column=2, padx=5, pady=2)
            
            # Melhor tempo
            best_label = tk.Label(score_grid, text="--:--", font=("Arial", 10), 
                                 fg="#00ff00", bg="#2b2b2b", width=12)
            best_label.grid(row=i+1, column=3, padx=5, pady=2)
            
            # Armazenar referências para atualização
            self.car_scores[i] = {
                'lap': lap_label,
                'time': time_label,
                'best': best_label,
                'start_time': 0,
                'last_lap_time': 0
            }
        
        # Botão para atualizar scores
        update_score_btn = tk.Button(score_frame, text="🔄 ATUALIZAR SCORES", 
                                    command=self.update_scores, bg="#0088aa", fg="white", 
                                    font=("Arial", 12, "bold"))
        update_score_btn.pack(pady=10)
        
        # === VISUALIZAÇÃO DA PISTA ===
        track_frame = ttk.Frame(score_frame)
        track_frame.pack(fill=tk.X, padx=20, pady=20)
        
        tk.Label(track_frame, text="🏁 VISUALIZAÇÃO DA PISTA:", font=("Arial", 12, "bold"), 
                fg="white", bg="#2b2b2b").pack()
        
        # Canvas para desenhar a pista
        self.track_canvas = tk.Canvas(track_frame, width=400, height=100, bg="#1a1a1a", 
                                     highlightthickness=0)
        self.track_canvas.pack(pady=10)
        
        # Desenhar pista base
        self.draw_track()
        
        # Botão para atualizar visualização
        update_track_btn = tk.Button(track_frame, text="🔄 ATUALIZAR PISTA", 
                                    command=self.update_track_visualization, bg="#aa0088", fg="white", 
                                    font=("Arial", 10, "bold"))
        update_track_btn.pack(pady=5)
        
        # === STATUS E DEBUG ===
        status_frame = tk.LabelFrame(config_frame, text="📊 STATUS E DEBUG", 
                                    font=("Arial", 14, "bold"), fg="#00ff00", bg="#2b2b2b")
        status_frame.pack(fill=tk.X, pady=(0, 20))
        
        # Botões de teste e status
        test_frame = ttk.Frame(status_frame)
        test_frame.pack(pady=20)
        
        status_btn = tk.Button(test_frame, text="📊 VER STATUS", 
                              command=lambda: self.send_command("s"), bg="#0088aa", fg="white", 
                              font=("Arial", 12, "bold"))
        status_btn.pack(side=tk.LEFT, padx=5)
        
        test_all_btn = tk.Button(test_frame, text="🧪 TESTAR TODOS OS CARROS", 
                                command=self.test_all_cars, bg="#aa0088", fg="white", 
                                font=("Arial", 12, "bold"))
        test_all_btn.pack(side=tk.LEFT, padx=5)
        
        test_car1_btn = tk.Button(test_frame, text="🔍 TESTAR CARRO 1", 
                                 command=self.test_car_1_specifically, bg="#ff8800", fg="white", 
                                 font=("Arial", 12, "bold"))
        test_car1_btn.pack(side=tk.LEFT, padx=5)
        
        telemetry_btn = tk.Button(test_frame, text="📡 TELEMETRIA", 
                                 command=lambda: self.send_command("p"), bg="#00aa88", fg="white", 
                                 font=("Arial", 12, "bold"))
        telemetry_btn.pack(side=tk.LEFT, padx=5)
        
        # Log de comandos
        log_frame = ttk.Frame(status_frame)
        log_frame.pack(fill=tk.BOTH, expand=True, padx=20, pady=(0, 20))
        
        tk.Label(log_frame, text="📝 LOG DE COMANDOS:", fg="white", bg="#2b2b2b").pack(anchor=tk.W)
        
        self.log_text = tk.Text(log_frame, height=8, bg="#1a1a1a", fg="#00ff00", 
                                font=("Consolas", 10))
        self.log_text.pack(fill=tk.BOTH, expand=True)
        
        # Scrollbar para o log
        scrollbar = tk.Scrollbar(self.log_text)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.log_text.config(yscrollcommand=scrollbar.set)
        scrollbar.config(command=self.log_text.yview)
        
        # Configurar eventos dos sliders
        self.accel_slider.config(command=self.update_accel_value)
        self.max_slider.config(command=self.update_max_value)
        self.init_slider.config(command=self.update_init_value)
        
        # Iniciar thread de monitoramento
        self.monitor_thread = threading.Thread(target=self.monitor_serial, daemon=True)
        self.monitor_thread.start()
        
        # Configurar evento de fechamento da janela
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
    
    def update_accel_value(self, value):
        """Atualizar valor da aceleração (1-50 = 0.01-0.50)"""
        self.accel_value.config(text=f"{float(value)/100:.2f}")
    
    def update_max_value(self, value):
        """Atualizar valor da velocidade máxima (5-100 = 0.5-10.0)"""
        self.max_value.config(text=f"{float(value)/10:.1f}")
    
    def update_init_value(self, value):
        """Atualizar valor da velocidade inicial (1-50 = 0.01-0.50)"""
        self.init_value.config(text=f"{float(value)/100:.2f}")
    
    def apply_config(self):
        """Aplicar configurações dos sliders"""
        if not self.ser:
            messagebox.showerror("Erro", "ESP não conectada!")
            return
        
        # Aplicar aceleração
        accel_val = self.accel_slider.get()
        self.send_command(f"a{accel_val}")
        
        # Aplicar velocidade máxima
        max_val = self.max_slider.get()
        self.send_command(f"m{max_val}")
        
        # Aplicar velocidade inicial
        init_val = self.init_slider.get()
        self.send_command(f"i{init_val}")
        
        self.log_message("⚡ Configurações aplicadas!")
    
    def test_all_cars(self):
        """Testar todos os carros para verificar funcionamento"""
        if not self.ser:
            messagebox.showerror("Erro", "ESP não conectada!")
            return
        
        self.log_message("🧪 Testando todos os carros...")
        
        # Primeiro iniciar corrida
        self.send_command("g")
        time.sleep(0.5)
        
        # Testar aceleração de todos os carros
        cars = ["a", "2", "d", "f"]  # Comandos corretos para cada carro
        car_names = ["Carro 1", "Carro 2", "Carro 3", "Carro 4"]
        
        for i, (cmd, name) in enumerate(zip(cars, car_names)):
            self.log_message(f"🚗 Testando {name} com comando '{cmd}'...")
            self.send_command(cmd)
            time.sleep(0.5)  # Mais tempo para ver resposta
            
            # Verificar se há resposta da ESP
            if self.ser.in_waiting:
                response = self.ser.readline().decode().strip()
                if response:
                    self.log_message(f"📡 {name} respondeu: {response}")
                else:
                    self.log_message(f"⚠️ {name} não respondeu")
            else:
                self.log_message(f"⚠️ {name} sem resposta")
        
        self.log_message("✅ Teste de todos os carros concluído!")
    
    def test_car_1_specifically(self):
        """Testar especificamente o carro 1 para debug"""
        if not self.ser:
            messagebox.showerror("Erro", "ESP não conectada!")
            return
        
        self.log_message("🔍 Testando especificamente o Carro 1...")
        
        # Verificar status atual
        self.log_message("📊 Verificando status atual...")
        self.send_command("s")
        time.sleep(1)
        
        # Iniciar corrida
        self.log_message("🏁 Iniciando corrida...")
        self.send_command("g")
        time.sleep(1)
        
        # Verificar status após iniciar
        self.log_message("📊 Verificando status após GO...")
        self.send_command("s")
        time.sleep(1)
        
        # Testar carro 1 várias vezes
        for i in range(3):
            self.log_message(f"🚗 Tentativa {i+1} - Carro 1 acelerando...")
            self.send_command("a")
            time.sleep(0.5)
            
            # Verificar resposta
            if self.ser.in_waiting:
                response = self.ser.readline().decode().strip()
                self.log_message(f"📡 Resposta: {response}")
            else:
                self.log_message("⚠️ Sem resposta")
        
        self.log_message("🔍 Teste específico do Carro 1 concluído!")
    
    def update_scores(self):
        """Atualizar scores dos carros via comando de status"""
        if not self.ser:
            messagebox.showerror("Erro", "ESP não conectada!")
            return
        
        self.log_message("📊 Atualizando scores...")
        self.send_command("s")
        
        # Simular atualização de scores (será implementado quando ESP retornar dados)
        for i in range(4):
            # Simular dados para teste
            lap = i + 1
            time_str = f"{lap:02d}:{lap*10:02d}"
            best_str = f"{lap-1:02d}:{(lap-1)*8:02d}" if lap > 1 else "--:--"
            
            self.car_scores[i]['lap'].config(text=str(lap))
            self.car_scores[i]['time'].config(text=time_str)
            self.car_scores[i]['best'].config(text=best_str)
        
        self.log_message("📊 Scores atualizados!")
    
    def draw_track(self):
        """Desenhar a pista base"""
        # Limpar canvas
        self.track_canvas.delete("all")
        
        # Desenhar pista retangular
        self.track_canvas.create_rectangle(10, 20, 390, 80, outline="#00ff00", width=2, fill="#0a0a0a")
        
        # Marcar posições importantes
        self.track_canvas.create_text(200, 10, text="🏁 LINHA DE CHEGADA", fill="#00ff00", font=("Arial", 8, "bold"))
        
        # Marcar posições dos carros (inicialmente na linha de partida)
        car_colors = ["#ffffff", "#00ff00", "#0000ff", "#ffff00"]  # Carro 1 agora é branco
        for i in range(4):
            x = 30 + (i * 20)
            y = 50
            self.track_canvas.create_oval(x-5, y-5, x+5, y+5, fill=car_colors[i], outline="white")
            self.track_canvas.create_text(x, y+15, text=f"C{i+1}", fill=car_colors[i], font=("Arial", 8, "bold"))
    
    def update_track_visualization(self):
        """Atualizar visualização da pista com posições atuais"""
        if not self.ser:
            messagebox.showerror("Erro", "ESP não conectada!")
            return
        
        self.log_message("🏁 Atualizando visualização da pista...")
        self.send_command("p")  # Solicitar telemetria
        
        # A visualização será atualizada automaticamente quando a telemetria chegar
        self.draw_track()
    
    def process_telemetry(self, data):
        """Processar dados de telemetria da ESP"""
        try:
            # Formato: p1T1,50,100 (carro 1, volta 1, posição 50, bateria 100)
            if data.startswith('p') and 'T' in data:
                parts = data[1:].split('T')  # Remove 'p' e divide por 'T'
                if len(parts) == 2:
                    car_num = int(parts[0]) - 1  # Carro 0-3
                    values = parts[1].split(',')
                    if len(values) >= 3 and 0 <= car_num < 4:
                        lap = int(values[0])
                        pos = int(values[1])
                        battery = int(values[2])
                        
                        # Atualizar interface
                        self.car_scores[car_num]['lap'].config(text=str(lap))
                        self.car_scores[car_num]['time'].config(text=f"{pos:02d}:{battery:02d}")
                        
                        # Atualizar melhor tempo se necessário
                        current_best = self.car_scores[car_num]['best'].cget('text')
                        if current_best == "--:--" or battery < int(current_best.split(':')[1]):
                            self.car_scores[car_num]['best'].config(text=f"{lap:02d}:{battery:02d}")
                        
                        # Atualizar visualização da pista
                        self.update_car_position(car_num, pos, lap)
                        
                        self.log_message(f"📡 Carro {car_num+1}: Volta {lap}, Pos {pos}, Bat {battery}")
        except Exception as e:
            self.log_message(f"❌ Erro ao processar telemetria: {e}")
    
    def update_car_position(self, car_num, pos, lap):
        """Atualizar posição do carro na visualização da pista"""
        try:
            # Calcular posição X na pista (400 pixels de largura)
            # Assumindo que a pista tem 200 LEDs, mapear posição para pixels
            track_width = 380  # Largura da pista no canvas
            x = 10 + (pos % 200) * track_width / 200
            
            # Calcular posição Y baseada na volta
            y = 30 + (lap - 1) * 15  # Cada volta em uma linha diferente
            
            # Limpar posição anterior do carro
            self.track_canvas.delete(f"car_{car_num}")
            
            # Cores dos carros
            car_colors = ["#ff00ff", "#00ff00", "#0000ff", "#ffff00"]  # Carro 1 é magenta
            
            # Desenhar carro na nova posição
            self.track_canvas.create_oval(x-8, y-8, x+8, y+8, 
                                        fill=car_colors[car_num], outline="white", 
                                        tags=f"car_{car_num}")
            
            # Adicionar número do carro
            self.track_canvas.create_text(x, y, text=f"{car_num+1}", 
                                        fill="black" if car_num == 3 else "white", 
                                        font=("Arial", 8, "bold"), 
                                        tags=f"car_{car_num}")
            
            # Adicionar indicador de volta
            self.track_canvas.create_text(x, y+12, text=f"V{lap}", 
                                        fill=car_colors[car_num], 
                                        font=("Arial", 6, "bold"), 
                                        tags=f"car_{car_num}")
            
        except Exception as e:
            self.log_message(f"❌ Erro ao atualizar posição do carro {car_num}: {e}")
    
    def log_message(self, message):
        """Adicionar mensagem ao log"""
        timestamp = time.strftime("%H:%M:%S")
        self.log_text.insert(tk.END, f"[{timestamp}] {message}\n")
        self.log_text.see(tk.END)
    
    def monitor_serial(self):
        """Monitorar respostas da ESP"""
        while self.running:
            if self.ser and self.ser.in_waiting:
                try:
                    # Ler todas as linhas disponíveis
                    while self.ser.in_waiting:
                        response = self.ser.readline().decode().strip()
                        if response:
                            # Processar telemetria automaticamente
                            if response.startswith('p') and 'T' in response:
                                self.process_telemetry(response)
                            else:
                                self.log_message(f"📡 ESP: {response}")
                except Exception as e:
                    self.log_message(f"❌ Erro na leitura: {e}")
            time.sleep(0.05)  # Mais rápido para melhor resposta
    
    def on_closing(self):
        """Função chamada quando a janela é fechada"""
        self.running = False
        if self.ser:
            self.ser.close()
        self.root.destroy()
    
    def run(self):
        """Executar a interface"""
        try:
            self.root.mainloop()
        except KeyboardInterrupt:
            self.running = False
            if self.ser:
                self.ser.close()
        except Exception as e:
            print(f"Erro na interface: {e}")
            self.running = False
            if self.ser:
                self.ser.close()

def main():
    # Verificar se porta foi especificada
    port = "/dev/cu.usbserial-110"
    if len(sys.argv) > 1:
        port = sys.argv[1]
    
    app = OLRGUIConfig(port=port)
    app.run()

if __name__ == "__main__":
    main()
