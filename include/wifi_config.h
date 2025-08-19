#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// ===== CONFIGURAÇÕES WIFI =====
const char* ssid = "VIVOFIBRA-WIFI6-2D81";
const char* password = "xgsxJmdzjFNro5q";

// ===== CONFIGURAÇÕES DO SISTEMA =====
#define HOSTNAME "olr-wifi-esp"
#define WEB_PORT 80

// ===== CONFIGURAÇÕES DE DEBUG =====
#define DEBUG_SERIAL true
#define DEBUG_WIFI true
#define DEBUG_GAME true

// ===== CONFIGURAÇÕES DE PERFORMANCE =====
#define PHYSICS_UPDATE_RATE 50    // 20 FPS
#define LED_UPDATE_RATE 50        // 20 FPS
#define TELEMETRY_RATE 1000       // 1 segundo

// ===== CONFIGURAÇÕES DE JOGO =====
#define MAX_PLAYERS 4
#define DEFAULT_LAPS 5
#define DEFAULT_TRACK_LENGTH 35

// ===== CONFIGURAÇÕES DE LED =====
#define LED_BRIGHTNESS 160
#define LED_DEFAULT_PIN 5

// ===== CONFIGURAÇÕES DE RAMPA =====
#define DEFAULT_RAMP_START 10
#define DEFAULT_RAMP_CENTER 15
#define DEFAULT_RAMP_END 20
#define DEFAULT_RAMP_HEIGHT 12

// ===== CONFIGURAÇÕES DE FÍSICA =====
#define DEFAULT_FRICTION 0.006
#define DEFAULT_GRAVITY 0.015

#endif
