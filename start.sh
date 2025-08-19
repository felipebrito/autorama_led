#!/bin/bash

# 🚀 Inicializador Simples - Autorama LED Race Game
# Script bash para macOS/Linux

echo "============================================================"
echo "🏁 AUTORAMA LED RACE GAME - INICIALIZADOR"
echo "============================================================"
echo "🎮 Jogo de corrida de LED para 4 jogadores"
echo "📱 Interface web existente"
echo "🔧 Servidor Python simples"
echo "============================================================"

# Verificar se Python está instalado
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 não encontrado!"
    echo "💡 Instale Python 3: https://python.org"
    exit 1
fi

echo "✅ Python 3 encontrado: $(python3 --version)"

# Verificar se os arquivos necessários existem
if [ ! -f "index.html" ]; then
    echo "❌ Arquivo index.html não encontrado!"
    echo "💡 Certifique-se de estar na pasta correta do projeto"
    exit 1
fi

if [ ! -f "script.js" ]; then
    echo "❌ Arquivo script.js não encontrado!"
    echo "💡 Certifique-se de estar na pasta correta do projeto"
    exit 1
fi

if [ ! -f "styles.css" ]; then
    echo "❌ Arquivo styles.css não encontrado!"
    echo "💡 Certifique-se de estar na pasta correta do projeto"
    exit 1
fi

echo "✅ Arquivos necessários encontrados"

# Tornar o script executável
chmod +x start_server.py

# Iniciar o sistema
echo ""
echo "🚀 Iniciando sistema..."
echo "💡 Pressione Ctrl+C para parar"
echo ""

# Executar o inicializador Python
python3 start_server.py

echo ""
echo "�� Sistema parado!"
