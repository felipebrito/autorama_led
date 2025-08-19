#!/usr/bin/env python3
"""
Servidor web robusto para a interface OLR ESP32
Comunica com a ESP via serial e serve a interface web
Versão melhorada para lidar com dados binários
"""

import serial
import json
import time
import threading
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import parse_qs, urlparse
import os

class OLRWebHandler(BaseHTTPRequestHandler):
    def __init__(self, *args, serial_connection=None, **kwargs):
        self.serial_connection = serial_connection
        super().__init__(*args, **kwargs)
    
    def do_GET(self):
        """Servir arquivos estáticos e rotas da API"""
        if self.path == '/':
            self.path = '/web_interface.html'
        
        # Rotas da API
        if self.path.startswith('/api/'):
            self.handle_api_get()
            return
        
        try:
            # Servir arquivos estáticos
            with open(self.path[1:], 'rb') as file:
                content = file.read()
                
            if self.path.endswith('.html'):
                content_type = 'text/html'
            elif self.path.endswith('.css'):
                content_type = 'text/css'
            elif self.path.endswith('.js'):
                content_type = 'application/javascript'
            else:
                content_type = 'text/plain'
                
            self.send_response(200)
            self.send_header('Content-type', content_type)
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(content)
            
        except FileNotFoundError:
            self.send_error(404, 'File not found')
        except Exception as e:
            self.send_error(500, f'Server error: {str(e)}')
    
    def handle_api_get(self):
        """Processar requisições GET da API"""
        if self.path == '/api/status':
            # Retornar status da ESP
            status = {
                'connected': self.serial_connection and self.serial_connection.is_open,
                'port': self.serial_connection.port if self.serial_connection else None,
                'timestamp': time.time()
            }
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps(status).encode())
        else:
            self.send_error(404, 'API endpoint not found')
    
    def do_POST(self):
        """Processar comandos da interface web"""
        try:
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data.decode('utf-8'))
            
            # Roteamento da API
            if self.path == '/api/test':
                self.handle_test_command(data)
                return
            elif self.path == '/api/command':
                self.handle_game_command(data)
                return
            else:
                self.send_error(404, 'API endpoint not found')
                return
                
        except Exception as e:
            print(f"❌ Erro ao processar comando: {e}")
            self.send_error(500, f'Error processing command: {str(e)}')
    
    def handle_test_command(self, data):
        """Processar comando de teste"""
        result = {
            'success': True,
            'message': 'Servidor funcionando',
            'timestamp': time.time()
        }
        
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(result).encode())
    
    def handle_game_command(self, data):
        """Processar comando do jogo"""
        command = data.get('command', '')
        print(f"📤 Comando: {command}")
        
        # Enviar comando para ESP
        if self.serial_connection and self.serial_connection.is_open:
            try:
                # Limpar buffer antes de enviar
                self.serial_connection.reset_input_buffer()
                
                # Enviar comando
                self.serial_connection.write(f"{command}\n".encode())
                time.sleep(0.2)  # Reduzido para melhor performance
                
                # Ler resposta com tratamento robusto de dados binários
                response = ""
                max_attempts = 5  # Reduzido para evitar loops longos
                attempts = 0
                
                while attempts < max_attempts:
                    if self.serial_connection.in_waiting:
                        try:
                            raw_data = self.serial_connection.readline()
                            if raw_data:
                                # Tentar diferentes codificações
                                line = None
                                
                                # Tentativa 1: UTF-8
                                try:
                                    line = raw_data.decode('utf-8').strip()
                                except UnicodeDecodeError:
                                    pass
                                
                                # Tentativa 2: Latin-1 (mais permissivo)
                                if line is None:
                                    try:
                                        line = raw_data.decode('latin-1').strip()
                                    except UnicodeDecodeError:
                                        pass
                                
                                # Tentativa 3: ASCII com ignorar erros
                                if line is None:
                                    try:
                                        line = raw_data.decode('ascii', errors='ignore').strip()
                                    except UnicodeDecodeError:
                                        pass
                                
                                # Se conseguiu decodificar, adicionar à resposta
                                if line and len(line) > 0:
                                    response += line + "\n"
                                    # Log apenas para comandos CRÍTICOS
                                    if command in ['g', 'r']:  # Apenas GO e RESET
                                        print(f"📡 ESP: {line}")
                                
                        except Exception as e:
                            print(f"⚠️ Erro ao ler linha: {e}")
                            break
                    
                    attempts += 1
                    time.sleep(0.02)  # Muito mais rápido (20ms)
                
                # Enviar resposta de volta
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                
                result = {
                    'success': True,
                    'command': command,
                    'response': response.strip(),
                    'timestamp': time.time()
                }
                
                self.wfile.write(json.dumps(result).encode())
                # Log apenas para comandos importantes
                if command in ['g', 'r', 't', 's']:
                    print(f"✅ {command} processado")
                
            except Exception as e:
                print(f"❌ Erro: {e}")
                self.send_response(500)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                
                result = {
                    'success': False,
                    'error': f'Erro interno: {str(e)}',
                    'timestamp': time.time()
                }
                
                self.wfile.write(json.dumps(result).encode())
        else:
            self.send_response(500)
            self.send_header('Content-type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            
            result = {
                'success': False,
                'error': 'Serial connection not available',
                'timestamp': time.time()
            }
            
            self.wfile.write(json.dumps(result).encode())
    
    def do_OPTIONS(self):
        """CORS preflight"""
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

class OLRWebServer:
    def __init__(self, port=8000, serial_port='/dev/cu.usbserial-110', baudrate=115200):
        self.port = port
        self.serial_port = serial_port
        self.baudrate = baudrate
        self.serial_connection = None
        self.server = None
        self.monitoring = False
        
    def connect_serial(self):
        """Conectar à ESP via serial"""
        try:
            self.serial_connection = serial.Serial(
                self.serial_port, 
                self.baudrate, 
                timeout=1
            )
            print(f"✅ Conectado à ESP via {self.serial_port}")
            return True
        except Exception as e:
            print(f"❌ Erro ao conectar serial: {e}")
            return False
    
    def start_monitoring(self):
        """Iniciar monitoramento da ESP em background - ULTRA OTIMIZADO"""
        if not self.serial_connection:
            return
            
        self.monitoring = True
        def monitor():
            last_log_time = 0
            log_interval = 10.0  # Log apenas a cada 10 segundos (muito menos frequente)
            telemetry_count = 0  # Contador para limitar logs
            
            while self.monitoring:
                try:
                    if self.serial_connection.in_waiting:
                        raw_data = self.serial_connection.readline()
                        if raw_data:
                            # Tentar diferentes codificações
                            line = None
                            
                            # Tentativa 1: UTF-8
                            try:
                                line = raw_data.decode('utf-8').strip()
                            except UnicodeDecodeError:
                                pass
                            
                            # Tentativa 2: Latin-1
                            if line is None:
                                try:
                                    line = raw_data.decode('latin-1').strip()
                                except UnicodeDecodeError:
                                    pass
                            
                            # Tentativa 3: ASCII com ignorar erros
                            if line is None:
                                try:
                                    line = raw_data.decode('ascii', errors='ignore').strip()
                                except UnicodeDecodeError:
                                    pass
                            
                            # Processar telemetria SILENCIOSAMENTE (sem log)
                            if line and len(line) > 0 and line.startswith('p') and 'T' in line:
                                telemetry_count += 1
                                
                                # Log apenas a cada 10 telemetrias ou a cada 10 segundos
                                current_time = time.time()
                                if (current_time - last_log_time > log_interval and telemetry_count > 50):
                                    print(f"📡 Telemetria processada: {telemetry_count} mensagens em {log_interval}s")
                                    last_log_time = current_time
                                    telemetry_count = 0
                except Exception as e:
                    print(f"❌ Erro no monitoramento: {e}")
                    break
                time.sleep(0.05)  # Muito mais rápido (50ms)
        
        thread = threading.Thread(target=monitor, daemon=True)
        thread.start()
        print("🔍 Monitoramento otimizado iniciado")
    
    def start_server(self):
        """Iniciar servidor web"""
        try:
            # Criar handler customizado com conexão serial
            def handler(*args, **kwargs):
                return OLRWebHandler(*args, serial_connection=self.serial_connection, **kwargs)
            
            self.server = HTTPServer(('localhost', self.port), handler)
            print(f"🌐 Servidor web iniciado em http://localhost:{self.port}")
            
            # Conectar serial
            if self.connect_serial():
                self.start_monitoring()
            
            # Iniciar servidor
            self.server.serve_forever()
            
        except KeyboardInterrupt:
            print("\n🛑 Servidor interrompido pelo usuário")
        except Exception as e:
            print(f"❌ Erro no servidor: {e}")
        finally:
            self.cleanup()
    
    def cleanup(self):
        """Limpar recursos"""
        if self.serial_connection:
            self.serial_connection.close()
            print("🔌 Conexão serial fechada")
        
        if self.server:
            self.server.shutdown()
            print("🌐 Servidor web parado")
        
        self.monitoring = False

def main():
    """Função principal"""
    print("🚀 Iniciando servidor web OLR ESP32 (versão robusta)...")
    
    # Configurações
    port = 8000
    serial_port = '/dev/cu.usbserial-110'  # Ajustar conforme necessário
    
    # Criar e iniciar servidor
    server = OLRWebServer(port=port, serial_port=serial_port)
    
    try:
        server.start_server()
    except KeyboardInterrupt:
        print("\n🛑 Servidor interrompido")
    finally:
        server.cleanup()

if __name__ == "__main__":
    main()
