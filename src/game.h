#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdlib.h> 
// O compilador já reconhece bool, true e false nativamente (C23) ou via raylib.h
// Portanto, não precisamos definir manualmente.

// --- Constantes Globais ---
#define LINHAS 20
#define COLUNAS 40
// Tamanho 40 para alta resolução (1600x840)
#define TAMANHO_BLOCO 40 

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

// --- Struct Mapa ---
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

// --- Struct Fantasma ---
typedef struct {
    Vector2 position; // Posição visual (pixels)
    Posicao gridPos;  // Posição lógica obrigatória (linha/coluna)
    
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
    
    int score;             
    int lives;             
    char rankingMessage[128]; 
    float saveMessageTimer;

    // Controle de Mute
    bool isMuted;

    // --- SONS ---
    Sound sfxIntro;     
    Sound sfxWaka;      
    Sound sfxEatGhost;  
    Sound sfxDeath;     

} Game;

// --- Protótipos Globais ---
Mapa* lerMapa(const char *arquivo);
void printarMapa(Mapa* mapa);
void descartarMapa(Mapa* mapa);

#endif // GAME_H