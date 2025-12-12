#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "game.h"


// Estados possíveis do jogo
typedef enum {
    SCREEN_MENU,        // Tela inicial
    SCREEN_PLAYING,     // Jogando
    SCREEN_GAME_OVER,   // Game Over
    SCREEN_VICTORY,     // Vitória (nível completo)
    SCREEN_NEXT_LEVEL   // Transição entre níveis
} GameScreen;

// Constantes de gameplay
#define INVULNERABILITY_TIME 2.0f  // Tempo de invulnerabilidade após morte (segundos)
#define GHOST_COLLISION_DISTANCE 30.0f  // Distância para detectar colisão
#define POINTS_PER_PELLET 10
#define POINTS_PER_POWER_PELLET 50
#define POINTS_PER_GHOST 100
#define PENALTY_DEATH 200  // Pontos perdidos ao morrer
#define INITIAL_LIVES 3
#define MAX_LEVEL 3

// Variáveis adicionais necessárias 
typedef struct {
    GameScreen currentScreen;
    float invulnerabilityTimer;
    bool isInvulnerable;
    float deathAnimationTimer;
    int totalPelletsInLevel;
    float transitionTimer;
} GameState;

// === FUNÇÕES PRINCIPAIS ===

// Inicializa o estado do jogo
void InitGameState(Game* game, GameState* state);

// Inicia um novo jogo (reseta tudo)
void StartNewGame(Game* game, GameState* state);

// Carrega o próximo nível
void LoadNextLevel(Game* game, GameState* state);

// Reseta a posição do Pac-Man após morte
void ResetPacmanPosition(Game* game);

// === DETECÇÃO DE COLISÕES ===

// Verifica colisão entre Pac-Man e Fantasmas
void CheckPacmanGhostCollision(Game* game, GameState* state);

// Verifica se Pac-Man coletou um pellet
void CheckPelletCollection(Game* game, GameState* state);

// === REGRAS DO JOGO ===

// Processa a morte do Pac-Man
void HandlePacmanDeath(Game* game, GameState* state);

// Verifica condições de vitória
bool CheckVictoryCondition(Game* game, GameState* state);

// Verifica condições de derrota
bool CheckDefeatCondition(Game* game, GameState* state);

// Atualiza o estado geral do jogo
void UpdateGameState(Game* game, GameState* state, float delta);

// === TELAS ===

// Desenha a tela inicial
void DrawMenuScreen(void);

// Desenha a tela de Game Over
void DrawGameOverScreen(Game* game);

// Desenha a tela de vitória
void DrawVictoryScreen(Game* game);

// Desenha a tela de transição de nível
void DrawLevelTransition(Game* game, GameState* state);

// Desenha o HUD (vidas, pontuação, nível)
void DrawHUD(Game* game, GameState* state);

#endif // GAMESTATE_H