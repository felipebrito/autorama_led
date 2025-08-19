#!/bin/bash

# 🚀 Script de Ativação e Execução - Autorama LED Race Game
# Ativa o ambiente virtual e executa o servidor

echo "============================================================"
echo "🏁 AUTORAMA LED RACE GAME - ATIVADOR AUTOMÁTICO"
echo "============================================================"

# Verificar se o ambiente virtual existe
if [ ! -d "autorama_env" ]; then
    echo "❌ Ambiente virtual não encontrado!"
    echo "💡 Execute primeiro: python3 -m venv autorama_env"
    exit 1
fi

# Ativar ambiente virtual
echo "🔧 Ativando ambiente virtual Python..."
source autorama_env/bin/activate

# Verificar se está ativo
if [[ "$VIRTUAL_ENV" != "" ]]; then
    echo "✅ Ambiente virtual ativo: $VIRTUAL_ENV"
    echo "🐍 Python: $(which python)"
    echo "📦 Pip: $(which pip)"
else
    echo "❌ Falha ao ativar ambiente virtual!"
    exit 1
fi

# Verificar dependências
echo ""
echo "🔍 Verificando dependências..."
python -c "import flask, serial, flask_cors; print('✅ Todas as dependências estão instaladas!')" 2>/dev/null

if [ $? -ne 0 ]; then
    echo "❌ Dependências faltando!"
    echo "📦 Instalando dependências..."
    pip install flask pyserial flask-cors
fi

# Executar servidor
echo ""
echo "🚀 Iniciando servidor..."
echo "💡 Pressione Ctrl+C para parar"
echo "============================================================"

python start_server.py
