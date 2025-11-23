#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdlib.h> 

#define LINHAS 20
#define COLUNAS 40
// #define TAMANHO_BLOCO 40 
#define TAMANHO_BLOCO 20

typedef enum {
    PAREDE,
    PELLET,
    PACMAN_INICIO,
    FANTASMA_INICIO,
    PORTAL,
    POWER_PELLET,
    VAZIO
} Blocos;

typedef struct {
    int x;
    int y;
} Posicao;

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

typedef struct {
    int x;
    int y;
} Direcao;


typedef struct {
    Vector2 position;
    Direcao direction;
    Color color;
    Color originalColor;
    bool podeDecidir;
    
    bool isVulnerable;
    float vulnerableTimer;
    
    bool isActive; 
} Ghost;

typedef struct {
    Posicao gridPos;
    Vector2 pixelPos;
    Direcao currentDir; // Adicionado: Direção atual
    Direcao nextDir;    // Adicionado: Buffer de entrada (para virar suavemente)
    int score;
    int lives;
    bool isPoweredUp;   // Adicionado: Se comeu o Power Pellet
    float powerUpTimer; // Adicionado: Tempo do efeito
} Pacman;

typedef struct {
    Mapa* mapa;            
    Pacman pacman;         
    Ghost* ghosts;         
    int ghostCount;        
    int currentLevel;
    bool isPaused;
    bool gameOver;
} Game;

Mapa* lerMapa(const char *arquivo);
void printarMapa(Mapa* mapa);
void descartarMapa(Mapa* mapa);

#endif 