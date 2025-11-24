#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdlib.h> 
#include <stdbool.h>

// --- Constantes Globais ---
#define LINHAS 20
#define COLUNAS 40
#define TAMANHO_BLOCO 20

// --- Enums ---
typedef enum {
    PAREDE,
    PELLET,
    PACMAN_INICIO,
    FANTASMA_INICIO,
    PORTAL,
    POWER_PELLET,
    VAZIO
} Blocos;

// --- Structs Básicas ---
typedef struct {
    int x;
    int y;
} Posicao; // Usado para Grid

typedef struct {
    int x;
    int y;
} Direcao;

// --- Struct Mapa (Membro 1) ---
typedef struct {
    Blocos** matriz;
    Posicao pacman_inicio;
    Posicao* fantasmas_inicio;
    Posicao* portais;
    int numFantasmas;
    int numPortais;
    int numPellets;
    int linhas;
    int colunas;
} Mapa;

// --- Struct Fantasma (Membro 3 - Você) ---
typedef struct {
    Vector2 position; // Posição em pixels
    Direcao direction;
    Color color;
    Color originalColor;
    bool podeDecidir;
    
    bool isVulnerable;
    float vulnerableTimer;
    
    bool isActive; 
} Ghost;

// --- Struct Pac ---
typedef struct {
    Posicao gridPos;
    Vector2 pixelPos;
    Posicao direcaoAtual;    
    Posicao proximaDirecao;
    int score;
    int lives;
} Pacman;

// --- Struct Principal Game ---
typedef struct {
    Mapa* mapa;            
    Pacman pacman;         
    Ghost* ghosts;         
    int ghostCount;        
    
    int currentLevel;
    bool isPaused;
    bool gameOver;
    
    int score;             // Pontuação global
    int lives;             // Vidas globais
    char rankingMessage[128]; 
    float saveMessageTimer;
} Game;

// --- Protótipos Globais (Membro 1) ---
Mapa* lerMapa(const char *arquivo);
void printarMapa(Mapa* mapa);
void descartarMapa(Mapa* mapa);

#endif // GAME_H