// Prevenção de inclusão múltipla  
#ifndef GAME_H
#define GAME_H

#include <raylib.h>  

// --- ESTRUTURAS DE DADOS ---
typedef struct {
    int row;
    int col;
} Position;

typedef struct {
    Position pos;
    // ...  
} Pacman;

typedef struct {
    Position pos;
    // ... 
} Ghost;

typedef struct {
    char** map;
    int map_rows;
    int map_cols;
    Pacman pacman;
    Ghost* ghosts;
    int ghost_count;
    // ...  
} GameState;


// --- ASSINATURAS DAS FUNÇÕES GLOBAIS ---
void InitGame(GameState *gs);
void UpdateGame(GameState *gs);
void DrawGame(const GameState *gs);
void UnloadGame(GameState *gs);


#endif // GAME_H