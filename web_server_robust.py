#!/usr/bin/env python3
"""
🏁 Servidor Web Robusto - Autorama LED Race Game
Servidor Flask para controle remoto via interface web moderna
"""

import os
import sys
import time
import json
import logging
from datetime import datetime
from flask import Flask, request, jsonify, render_template_string, send_from_directory
from flask_cors import CORS
import serial
import serial.tools.list_ports

# Configuração de logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('autorama_server.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

# Configuração da aplicação Flask
app = Flask(__name__)
CORS(app)  # Permitir CORS para desenvolvimento

# Configurações globais
SERIAL_PORT = None
SERIAL_CONNECTION = None
CONNECTION_STATUS = False
LAST_TELEMETRY = {}
GAME_STATE = {
    'status': 'stopped',
    'start_time': None,
    'cars': {
        1: {'position': 0, 'speed': 0.0, 'laps': 0, 'last_update': None},
        2: {'position': 0, 'speed': 0.0, 'laps': 0, 'last_update': None},
        3: {'position': 0, 'speed': 0.0, 'laps': 0, 'last_update': None},
        4: {'position': 0, 'speed': 0.0, 'laps': 0, 'last_update': None}
    }
}

# Configurações de velocidade
SPEED_CONFIG = {
    'acceleration_rate': 0.2,
    'max_speed': 5.0,
    'initial_speed': 0.5
}

class SerialManager:
    """Gerenciador de conexão serial com ESP32"""
    
    def __init__(self):
        self.port = None
        self.connection = None
        self.connected = False
        
    def list_ports(self):
        """Lista todas as portas seriais disponíveis"""
        try:
            ports = list(serial.tools.list_ports.comports())
            port_info = []
            for port in ports:
                port_info.append({
                    'device': port.device,
                    'description': port.description,
                    'manufacturer': port.manufacturer,
                    'hwid': port.hwid
                })
            return port_info
        except Exception as e:
            logger.error(f"Erro ao listar portas: {e}")
            return []
    
    def connect(self, port_name):
        """Conecta a uma porta serial específica"""
        try:
            if self.connection:
                self.disconnect()
            
            logger.info(f"Conectando à porta {port_name}...")
            self.connection = serial.Serial(
                port=port_name,
                baudrate=115200,
                timeout=1,
                write_timeout=1
            )
            
            # Aguardar inicialização
            time.sleep(2)
            
            if self.connection.is_open:
                self.port = port_name
                self.connected = True
                logger.info(f"Conectado com sucesso à porta {port_name}")
                
                # Testar comunicação
                self.send_command('s')  # Status
                return True
            else:
                logger.error("Falha ao abrir conexão serial")
                return False
                
        except Exception as e:
            logger.error(f"Erro ao conectar: {e}")
            self.connected = False
            return False
    
    def disconnect(self):
        """Desconecta da porta serial"""
        try:
            if self.connection and self.connection.is_open:
                self.connection.close()
            self.connection = None
            self.port = None
            self.connected = False
            logger.info("Desconectado da porta serial")
            return True
        except Exception as e:
            logger.error(f"Erro ao desconectar: {e}")
            return False
    
    def send_command(self, command):
        """Envia comando para o ESP32"""
        if not self.connected or not self.connection:
            logger.warning("Não conectado ao ESP32")
            return None
        
        try:
            # Adicionar quebra de linha se necessário
            if not command.endswith('\n'):
                command += '\n'
            
            logger.info(f"Enviando comando: {command.strip()}")
            self.connection.write(command.encode('utf-8'))
            
            # Aguardar resposta
            time.sleep(0.1)
            
            if self.connection.in_waiting:
                response = self.connection.readline().decode('utf-8').strip()
                logger.info(f"Resposta recebida: {response}")
                return response
            else:
                logger.info("Comando enviado, sem resposta")
                return "OK"
                
        except Exception as e:
            logger.error(f"Erro ao enviar comando: {e}")
            return None
    
    def read_telemetry(self):
        """Lê telemetria do ESP32"""
        if not self.connected or not self.connection:
            return None
        
        try:
            if self.connection.in_waiting:
                data = self.connection.readline().decode('utf-8').strip()
                if data:
                    logger.debug(f"Telemetria recebida: {data}")
                    return data
            return None
        except Exception as e:
            logger.error(f"Erro ao ler telemetria: {e}")
            return None

# Instância global do gerenciador serial
serial_manager = SerialManager()

# Rotas da API
@app.route('/')
def index():
    """Página principal - redireciona para a interface moderna"""
    return render_template_string('''
        <!DOCTYPE html>
        <html>
        <head>
            <title>🏁 Autorama LED Race Game</title>
            <meta http-equiv="refresh" content="0; url=/web_interface.html">
        </head>
        <body>
            <p>Redirecionando para a interface moderna...</p>
        </body>
        </html>
    ''')

@app.route('/web_interface.html')
def serve_web_interface():
    """Serve a interface web moderna"""
    try:
        response = send_from_directory('.', 'web_interface.html')
        response.headers['Cache-Control'] = 'no-cache, no-store, must-revalidate'
        response.headers['Pragma'] = 'no-cache'
        response.headers['Expires'] = '0'
        return response
    except Exception as e:
        logger.error(f"Erro ao servir interface web: {e}")
        return f"Erro ao carregar interface: {e}", 500

@app.route('/<path:filename>')
def serve_static(filename):
    """Serve arquivos estáticos"""
    try:
        return send_from_directory('.', filename)
    except Exception as e:
        logger.error(f"Erro ao servir arquivo {filename}: {e}")
        return f"Arquivo não encontrado: {filename}", 404

@app.route('/api/status')
def get_status():
    """Retorna status da conexão e do jogo"""
    return jsonify({
        'connected': serial_manager.connected,
        'port': serial_manager.port,
        'game_state': GAME_STATE,
        'speed_config': SPEED_CONFIG,
        'timestamp': datetime.now().isoformat()
    })

@app.route('/api/ports')
def get_ports():
    """Lista portas seriais disponíveis"""
    ports = serial_manager.list_ports()
    return jsonify({
        'ports': ports,
        'count': len(ports)
    })

@app.route('/api/connect', methods=['POST'])
def connect_serial():
    """Conecta a uma porta serial"""
    try:
        data = request.get_json()
        port_name = data.get('port')
        
        if not port_name:
            return jsonify({'success': False, 'error': 'Porta não especificada'}), 400
        
        if serial_manager.connect(port_name):
            global SERIAL_PORT, SERIAL_CONNECTION, CONNECTION_STATUS
            SERIAL_PORT = port_name
            SERIAL_CONNECTION = serial_manager.connection
            CONNECTION_STATUS = True
            
            return jsonify({
                'success': True,
                'message': f'Conectado à porta {port_name}',
                'port': port_name
            })
        else:
            return jsonify({
                'success': False,
                'error': f'Falha ao conectar à porta {port_name}'
            }), 500
            
    except Exception as e:
        logger.error(f"Erro na conexão: {e}")
        return jsonify({
            'success': False,
            'error': str(e)
        }), 500

@app.route('/api/disconnect', methods=['POST'])
def disconnect_serial():
    """Desconecta da porta serial"""
    try:
        if serial_manager.disconnect():
            global SERIAL_PORT, SERIAL_CONNECTION, CONNECTION_STATUS
            SERIAL_PORT = None
            SERIAL_CONNECTION = None
            CONNECTION_STATUS = False
            
            return jsonify({
                'success': True,
                'message': 'Desconectado com sucesso'
            })
        else:
            return jsonify({
                'success': False,
                'error': 'Falha ao desconectar'
            }), 500
            
    except Exception as e:
        logger.error(f"Erro na desconexão: {e}")
        return jsonify({
            'success': False,
            'error': str(e)
        }), 500

@app.route('/api/command', methods=['POST'])
def send_command():
    """Envia comando para o ESP32"""
    try:
        data = request.get_json()
        command = data.get('command')
        
        if not command:
            return jsonify({'success': False, 'error': 'Comando não especificado'}), 400
        
        if not serial_manager.connected:
            return jsonify({'success': False, 'error': 'Não conectado ao ESP32'}), 400
        
        # Processar comando especial
        if command == 'g':  # GO
            GAME_STATE['status'] = 'running'
            GAME_STATE['start_time'] = datetime.now()
            logger.info("Jogo iniciado")
        elif command == 'r':  # RESET
            GAME_STATE['status'] = 'stopped'
            GAME_STATE['start_time'] = None
            # Resetar posições dos carros
            for car in GAME_STATE['cars'].values():
                car['position'] = 0
                car['speed'] = 0.0
                car['laps'] = 0
            logger.info("Jogo resetado")
        
        # Enviar comando para ESP32
        response = serial_manager.send_command(command)
        
        if response is not None:
            return jsonify({
                'success': True,
                'command': command,
                'response': response,
                'game_state': GAME_STATE
            })
        else:
            return jsonify({
                'success': False,
                'error': 'Falha na comunicação com ESP32'
            }), 500
            
    except Exception as e:
        logger.error(f"Erro ao enviar comando: {e}")
        return jsonify({
            'success': False,
            'error': str(e)
        }), 500

@app.route('/api/config', methods=['POST'])
def update_config():
    """Atualiza configurações de velocidade"""
    try:
        data = request.get_json()
        
        # Atualizar configurações
        if 'acceleration_rate' in data:
            SPEED_CONFIG['acceleration_rate'] = float(data['acceleration_rate'])
        if 'max_speed' in data:
            SPEED_CONFIG['max_speed'] = float(data['max_speed'])
        if 'initial_speed' in data:
            SPEED_CONFIG['initial_speed'] = float(data['initial_speed'])
        
        logger.info(f"Configurações atualizadas: {SPEED_CONFIG}")
        
        return jsonify({
            'success': True,
            'config': SPEED_CONFIG,
            'message': 'Configurações atualizadas'
        })
        
    except Exception as e:
        logger.error(f"Erro ao atualizar configurações: {e}")
        return jsonify({
            'success': False,
            'error': str(e)
        }), 500

@app.route('/api/telemetry')
def get_telemetry():
    """Retorna telemetria atual dos carros"""
    return jsonify({
        'cars': GAME_STATE['cars'],
        'game_status': GAME_STATE['status'],
        'timestamp': datetime.now().isoformat()
    })

@app.route('/api/test')
def test_connection():
    """Testa conexão com o servidor"""
    return jsonify({
        'success': True,
        'message': 'Servidor funcionando',
        'timestamp': datetime.now().isoformat(),
        'version': '1.0.0'
    })

# Função para processar telemetria em background
def process_telemetry():
    """Processa telemetria recebida do ESP32"""
    while True:
        try:
            if serial_manager.connected:
                data = serial_manager.read_telemetry()
                if data:
                    # Processar telemetria no formato: p1T1,50,100
                    if data.startswith('p') and 'T' in data:
                        try:
                            parts = data.split('T')
                            car_num = int(parts[0][1:])  # p1 -> 1
                            telemetry_parts = parts[1].split(',')
                            
                            if len(telemetry_parts) >= 3:
                                lap = int(telemetry_parts[0])
                                position = int(telemetry_parts[1])
                                battery = int(telemetry_parts[2])
                                
                                if car_num in GAME_STATE['cars']:
                                    GAME_STATE['cars'][car_num].update({
                                        'position': position,
                                        'laps': lap,
                                        'last_update': datetime.now().isoformat()
                                    })
                                    
                                    logger.debug(f"Carro {car_num}: L{lap}, P{position}, B{battery}")
                        except Exception as e:
                            logger.warning(f"Erro ao processar telemetria: {e}")
            
            time.sleep(0.1)  # 100ms entre verificações
            
        except Exception as e:
            logger.error(f"Erro no processamento de telemetria: {e}")
            time.sleep(1)

# Inicialização
def init_server():
    """Inicializa o servidor"""
    logger.info("🏁 Iniciando servidor Autorama LED Race Game...")
    
    # Verificar se a interface web existe
    if not os.path.exists('web_interface.html'):
        logger.error("Interface web não encontrada!")
        logger.error("Certifique-se de que web_interface.html está na pasta atual")
        return False
    
    logger.info("✅ Interface web encontrada")
    logger.info("✅ Servidor Flask configurado")
    logger.info("✅ Gerenciador serial inicializado")
    
    return True

# Inicializar servidor
if __name__ == '__main__':
    if not init_server():
        sys.exit(1)
    
    try:
        # Iniciar processamento de telemetria em background
        import threading
        telemetry_thread = threading.Thread(target=process_telemetry, daemon=True)
        telemetry_thread.start()
        
        logger.info("🚀 Servidor iniciado em http://localhost:8000")
        logger.info("🌐 Interface web disponível em http://localhost:8000/web_interface.html")
        logger.info("📡 API disponível em http://localhost:8000/api/")
        logger.info("🛑 Pressione Ctrl+C para parar")
        
        # Executar servidor Flask
        app.run(host='0.0.0.0', port=8000, debug=False)
        
    except KeyboardInterrupt:
        logger.info("\n🛑 Servidor parado pelo usuário")
        if serial_manager.connected:
            serial_manager.disconnect()
        logger.info("👋 Até logo!")
    except Exception as e:
        logger.error(f"❌ Erro fatal: {e}")
        sys.exit(1)
