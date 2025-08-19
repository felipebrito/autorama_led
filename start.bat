@echo off
chcp 65001 >nul
title Autorama LED Race Game - Inicializador

echo ============================================================
echo 🏁 AUTORAMA LED RACE GAME - INICIALIZADOR
echo ============================================================
echo 🎮 Jogo de corrida de LED para 4 jogadores
echo 📱 Interface web existente
echo 🔧 Servidor Python simples
echo ============================================================

REM Verificar se Python está instalado
python --version >nul 2>&1
if errorlevel 1 (
    echo ❌ Python não encontrado!
    echo 💡 Instale Python: https://python.org
    pause
    exit /b 1
)

echo ✅ Python encontrado:
python --version

REM Verificar se os arquivos necessários existem
if not exist "index.html" (
    echo ❌ Arquivo index.html não encontrado!
    echo 💡 Certifique-se de estar na pasta correta do projeto
    pause
    exit /b 1
)

if not exist "script.js" (
    echo ❌ Arquivo script.js não encontrado!
    echo 💡 Certifique-se de estar na pasta correta do projeto
    pause
    exit /b 1
)

if not exist "styles.css" (
    echo ❌ Arquivo styles.css não encontrado!
    echo 💡 Certifique-se de estar na pasta correta do projeto
    pause
    exit /b 1
)

echo ✅ Arquivos necessários encontrados

echo.
echo 🚀 Iniciando sistema...
echo 💡 Pressione Ctrl+C para parar
echo.

REM Executar o inicializador Python
python start_server.py

echo.
echo 👋 Sistema parado!
pause
