#!/usr/bin/env python3
"""
🚀 Inicializador Moderno - Autorama LED Race Game
Inicia o servidor Flask e abre a interface web moderna
"""

import os
import sys
import time
import webbrowser
import subprocess
from pathlib import Path

def print_banner():
    """Exibe banner do projeto"""
    print("=" * 60)
    print("🏁 AUTORAMA LED RACE GAME - INICIALIZADOR MODERNO")
    print("=" * 60)
    print("🎮 Jogo de corrida de LED para 4 jogadores")
    print("📱 Interface web moderna com Neo Brutalism UI")
    print("🔧 Servidor Flask robusto para controle remoto")
    print("=" * 60)

def check_files():
    """Verifica se os arquivos necessários existem"""
    print("🔍 Verificando arquivos...")
    
    required_files = ['web_interface.html', 'web_server_robust.py']
    missing_files = []
    
    for file in required_files:
        if Path(file).exists():
            print(f"✅ {file} - OK")
        else:
            missing_files.append(file)
            print(f"❌ {file} - FALTANDO")
    
    if missing_files:
        print(f"\n⚠️  Arquivos faltando: {', '.join(missing_files)}")
        return False
    
    return True

def check_dependencies():
    """Verifica se as dependências estão instaladas"""
    print("\n🔍 Verificando dependências...")
    
    required_packages = ['flask', 'pyserial', 'flask-cors']
    missing_packages = []
    
    for package in required_packages:
        try:
            __import__(package.replace('-', '_'))
            print(f"✅ {package} - OK")
        except ImportError:
            missing_packages.append(package)
            print(f"❌ {package} - FALTANDO")
    
    if missing_packages:
        print(f"\n⚠️  Dependências faltando: {', '.join(missing_packages)}")
        print("📦 Instalando dependências...")
        
        try:
            subprocess.check_call([sys.executable, '-m', 'pip', 'install'] + missing_packages)
            print("✅ Dependências instaladas com sucesso!")
        except subprocess.CalledProcessError:
            print("❌ Erro ao instalar dependências!")
            print("💡 Execute manualmente: pip install flask pyserial flask-cors")
            return False
    
    return True

def start_flask_server():
    """Inicia o servidor Flask"""
    print("\n🌐 Iniciando servidor Flask...")
    
    try:
        # Iniciar servidor Flask em background
        process = subprocess.Popen([
            sys.executable, "web_server_robust.py"
        ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        
        # Aguardar um pouco para o servidor inicializar
        time.sleep(3)
        
        # Verificar se o processo está rodando
        if process.poll() is None:
            print("✅ Servidor Flask iniciado com sucesso!")
            print("🌐 URL: http://localhost:8000")
            print("📱 Interface moderna: http://localhost:8000/web_interface.html")
            return process
        else:
            stdout, stderr = process.communicate()
            print("❌ Erro ao iniciar servidor Flask:")
            print(stderr.decode())
            return False
            
    except Exception as e:
        print(f"❌ Erro ao iniciar servidor Flask: {e}")
        return False

def open_browser():
    """Abre o navegador automaticamente"""
    print("\n🌐 Abrindo navegador...")
    
    try:
        # Aguardar um pouco mais para o servidor estar pronto
        time.sleep(2)
        
        # Abrir navegador na interface moderna
        webbrowser.open('http://localhost:8000/web_interface.html')
        print("✅ Navegador aberto na interface moderna!")
        
    except Exception as e:
        print(f"⚠️  Não foi possível abrir o navegador automaticamente: {e}")
        print("💡 Abra manualmente: http://localhost:8000/web_interface.html")

def show_controls():
    """Mostra controles disponíveis"""
    print("\n🎮 COMO JOGAR:")
    print("=" * 40)
    print("🌐 Interface Web Moderna:")
    print("   • Acesse: http://localhost:8000/web_interface.html")
    print("   • Use os botões para controlar os carros")
    print("   • Visualize a pista em tempo real")
    print("   • Configure parâmetros do jogo")
    print()
    print("🔧 Controle Serial (ESP32):")
    print("   • Conecte o ESP32 via USB")
    print("   • Use o botão 'Conectar ESP32' na interface")
    print("   • Comandos: g (GO), a/2/d/f (carros), s (status)")
    print()
    print("📡 API REST disponível:")
    print("   • Status: http://localhost:8000/api/status")
    print("   • Portas: http://localhost:8000/api/ports")
    print("   • Comandos: http://localhost:8000/api/command")
    print()
    print("🛑 Para parar o servidor: Ctrl+C")

def main():
    """Função principal"""
    try:
        # Banner
        print_banner()
        
        # Verificar arquivos
        if not check_files():
            print("\n❌ Arquivos necessários não encontrados!")
            print("💡 Certifique-se de estar na pasta correta do projeto")
            return
        
        # Verificar dependências
        if not check_dependencies():
            return
        
        # Iniciar servidor
        server_process = start_flask_server()
        if not server_process:
            return
        
        # Abrir navegador
        open_browser()
        
        # Mostrar controles
        show_controls()
        
        # Manter o script rodando
        print("\n🚀 Sistema iniciado! Pressione Ctrl+C para parar...")
        print("=" * 60)
        
        try:
            # Aguardar interrupção
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            print("\n\n🛑 Parando servidor...")
            if server_process:
                server_process.terminate()
                server_process.wait()
            print("✅ Servidor parado!")
            print("👋 Até logo!")
        
    except Exception as e:
        print(f"\n❌ Erro inesperado: {e}")
        print("💡 Verifique se todos os arquivos estão presentes")

if __name__ == "__main__":
    main()
