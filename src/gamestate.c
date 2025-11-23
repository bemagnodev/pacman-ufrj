#include "gamestate.h"
#include "ghost.h"
#include "persistence.h"
#include "pacman.h"  // ← IMPORTANTE: Adicionar este include!
#include <stdio.h>
#include <math.h>
#include <string.h>

// === FUNÇÕES DE INICIALIZAÇÃO ===

void InitGameState(Game* game, GameState* state) {
    state->currentScreen = SCREEN_MENU;
    state->invulnerabilityTimer = 0.0f;
    state->isInvulnerable = false;
    state->deathAnimationTimer = 0.0f;
    state->totalPelletsInLevel = 0;
    state->transitionTimer = 0.0f;
    
    game->score = 0;
    game->lives = INITIAL_LIVES;
    game->currentLevel = 1;
    game->gameOver = false;
    game->isPaused = false;
}

void StartNewGame(Game* game, GameState* state) {
    // Limpa o mapa antigo se existir
    if (game->mapa != NULL) {
        descartarMapa(game->mapa);
    }
    
    // Limpa os fantasmas antigos
    UnloadGhosts(game);
    
    // Reinicia valores
    game->score = 0;
    game->lives = INITIAL_LIVES;
    game->currentLevel = 1;
    game->gameOver = false;
    game->isPaused = false;
    
    state->invulnerabilityTimer = 0.0f;
    state->isInvulnerable = false;
    state->deathAnimationTimer = 0.0f;
    state->transitionTimer = 0.0f;
    
    // Carrega o mapa
    game->mapa = lerMapa("nivel/mapa.txt");
    
    if (game->mapa != NULL) {
        // Inicializa o Pac-Man (CRÍTICO!)
        InitPacman(game);
        
        game->pacman.score = 0;
        game->pacman.lives = INITIAL_LIVES;
        
        // Inicializa fantasmas
        InitGhosts(game);
        
        // Conta pellets totais
        state->totalPelletsInLevel = game->mapa->numPellets;
    }
    
    state->currentScreen = SCREEN_PLAYING;
}

void LoadNextLevel(Game* game, GameState* state) {
    game->currentLevel++;
    
    // Se passou do último nível, volta ao primeiro (loop)
    if (game->currentLevel > MAX_LEVEL) {
        game->currentLevel = 1;
    }
    
    // Limpa o mapa e fantasmas atuais
    if (game->mapa != NULL) {
        descartarMapa(game->mapa);
    }
    UnloadGhosts(game);
    
    // Recarrega o mapa
    game->mapa = lerMapa("nivel/mapa.txt");
    
    if (game->mapa != NULL) {
        // Reinicia Pac-Man (CRÍTICO!)
        InitPacman(game);
        
        // Reinicia fantasmas
        InitGhosts(game);
        
        // Atualiza pellets totais
        state->totalPelletsInLevel = game->mapa->numPellets;
    }
    
    state->invulnerabilityTimer = 0.0f;
    state->isInvulnerable = false;
    state->currentScreen = SCREEN_PLAYING;
}

void ResetPacmanPosition(Game* game) {
    if (game->mapa == NULL) return;
    
    // Reinicia o Pac-Man (CRÍTICO!)
    InitPacman(game);
    
    // Reinicia os fantasmas nas posições iniciais
    for (int i = 0; i < game->ghostCount; i++) {
        int gridX = game->mapa->fantasmas_inicio[i].x;
        int gridY = game->mapa->fantasmas_inicio[i].y;
        
        game->ghosts[i].position.x = gridX * TAMANHO_BLOCO;
        game->ghosts[i].position.y = gridY * TAMANHO_BLOCO;
        game->ghosts[i].isActive = true;
        game->ghosts[i].isVulnerable = false;
        game->ghosts[i].vulnerableTimer = 0.0f;
        game->ghosts[i].color = game->ghosts[i].originalColor;
    }
}

// === DETECÇÃO DE COLISÕES ===

void CheckPacmanGhostCollision(Game* game, GameState* state) {
    if (state->isInvulnerable) return;
    if (game->mapa == NULL) return;
    
    float pacmanX = game->pacman.pixelPos.x + (TAMANHO_BLOCO / 2);
    float pacmanY = game->pacman.pixelPos.y + (TAMANHO_BLOCO / 2);
    
    for (int i = 0; i < game->ghostCount; i++) {
        if (!game->ghosts[i].isActive) continue;
        
        float ghostX = game->ghosts[i].position.x + (TAMANHO_BLOCO / 2);
        float ghostY = game->ghosts[i].position.y + (TAMANHO_BLOCO / 2);
        
        // Calcula distância
        float dx = pacmanX - ghostX;
        float dy = pacmanY - ghostY;
        float distance = sqrtf(dx * dx + dy * dy);
        
        // Se colidiu
        if (distance < GHOST_COLLISION_DISTANCE) {
            if (game->ghosts[i].isVulnerable) {
                // Pac-Man come o fantasma
                game->ghosts[i].isActive = false;
                game->score += POINTS_PER_GHOST;
            } else {
                // Fantasma mata o Pac-Man
                HandlePacmanDeath(game, state);
                return;
            }
        }
    }
}

void CheckPelletCollection(Game* game, GameState* state) {
    if (game->mapa == NULL) return;
    
    int gridX = (int)((game->pacman.pixelPos.x + TAMANHO_BLOCO / 2) / TAMANHO_BLOCO);
    int gridY = (int)((game->pacman.pixelPos.y + TAMANHO_BLOCO / 2) / TAMANHO_BLOCO);
    
    // Verifica limites
    if (gridX < 0 || gridX >= game->mapa->colunas) return;
    if (gridY < 0 || gridY >= game->mapa->linhas) return;
    
    Blocos tileAtual = game->mapa->matriz[gridY][gridX];
    
    // Coletou pellet normal
    if (tileAtual == PELLET) {
        game->mapa->matriz[gridY][gridX] = VAZIO;
        game->score += POINTS_PER_PELLET;
        game->mapa->numPellets--;
    }
    // Coletou power pellet
    else if (tileAtual == POWER_PELLET) {
        game->mapa->matriz[gridY][gridX] = VAZIO;
        game->score += POINTS_PER_POWER_PELLET;
        ScareGhosts(game);
    }
}

// === REGRAS DO JOGO ===

void HandlePacmanDeath(Game* game, GameState* state) {
    game->lives--;
    
    // Penalidade de pontos
    game->score -= PENALTY_DEATH;
    if (game->score < 0) game->score = 0;
    
    if (game->lives <= 0) {
        // Game Over
        game->gameOver = true;
        state->currentScreen = SCREEN_GAME_OVER;
        
        // Atualiza o ranking
        UpdateAndSaveRanking(game);
    } else {
        // Ainda tem vidas, reseta posição
        ResetPacmanPosition(game);
        
        // Ativa invulnerabilidade temporária
        state->isInvulnerable = true;
        state->invulnerabilityTimer = INVULNERABILITY_TIME;
        state->deathAnimationTimer = 0.5f;
    }
}

bool CheckVictoryCondition(Game* game, GameState* state) {
    if (game->mapa == NULL) return false;
    
    // Vitória: todos os pellets coletados
    if (game->mapa->numPellets <= 0) {
        state->currentScreen = SCREEN_VICTORY;
        state->transitionTimer = 3.0f;
        return true;
    }
    
    return false;
}

bool CheckDefeatCondition(Game* game, GameState* state) {
    return game->lives <= 0;
}

void UpdateGameState(Game* game, GameState* state, float delta) {
    // Atualiza timer de invulnerabilidade
    if (state->isInvulnerable) {
        state->invulnerabilityTimer -= delta;
        if (state->invulnerabilityTimer <= 0.0f) {
            state->isInvulnerable = false;
        }
    }
    
    // Atualiza timer de animação de morte
    if (state->deathAnimationTimer > 0.0f) {
        state->deathAnimationTimer -= delta;
    }
    
    // Atualiza timer de transição
    if (state->transitionTimer > 0.0f) {
        state->transitionTimer -= delta;
        
        if (state->transitionTimer <= 0.0f) {
            // Acabou a transição, carrega próximo nível
            if (state->currentScreen == SCREEN_VICTORY) {
                LoadNextLevel(game, state);
            }
        }
    }
    
    // Lógica específica de cada tela
    switch (state->currentScreen) {
        case SCREEN_PLAYING:
            // Verifica colisões
            CheckPacmanGhostCollision(game, state);
            CheckPelletCollection(game, state);
            
            // Verifica condições de vitória/derrota
            if (!CheckVictoryCondition(game, state)) {
                CheckDefeatCondition(game, state);
            }
            break;
            
        case SCREEN_MENU:
            // Input da tela inicial
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                StartNewGame(game, state);
            }
            break;
            
        case SCREEN_GAME_OVER:
            // Input do Game Over
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                state->currentScreen = SCREEN_MENU;
            }
            break;
            
        case SCREEN_VICTORY:
            // Aguarda o timer de transição
            break;
            
        default:
            break;
    }
}

// === FUNÇÕES DE DESENHO ===

void DrawMenuScreen(void) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    ClearBackground(BLACK);
    
    const char* titulo = "PAC-MAN";
    int larguraTitulo = MeasureText(titulo, 60);
    DrawText(titulo, screenWidth / 2 - larguraTitulo / 2, screenHeight / 2 - 100, 60, YELLOW);
    
    const char* instrucao = "Pressione ENTER para Iniciar";
    int larguraInstrucao = MeasureText(instrucao, 20);
    DrawText(instrucao, screenWidth / 2 - larguraInstrucao / 2, screenHeight / 2, 20, WHITE);
    
    DrawText("Controles:", screenWidth / 2 - 100, screenHeight / 2 + 60, 20, LIGHTGRAY);
    DrawText("Setas: Mover", screenWidth / 2 - 100, screenHeight / 2 + 90, 18, GRAY);
    DrawText("TAB: Menu de Pausa", screenWidth / 2 - 100, screenHeight / 2 + 115, 18, GRAY);
    
    DrawText("Trabalho em Grupo - Programacao 2", 10, screenHeight - 25, 15, DARKGRAY);
}

void DrawGameOverScreen(Game* game) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 200});
    
    const char* titulo = "GAME OVER";
    int larguraTitulo = MeasureText(titulo, 50);
    DrawText(titulo, screenWidth / 2 - larguraTitulo / 2, screenHeight / 2 - 100, 50, RED);
    
    char scoreText[64];
    sprintf(scoreText, "Pontuacao Final: %d", game->score);
    int larguraScore = MeasureText(scoreText, 30);
    DrawText(scoreText, screenWidth / 2 - larguraScore / 2, screenHeight / 2 - 20, 30, WHITE);
    
    if (strlen(game->rankingMessage) > 0) {
        int larguraRanking = MeasureText(game->rankingMessage, 20);
        DrawText(game->rankingMessage, screenWidth / 2 - larguraRanking / 2, screenHeight / 2 + 30, 20, YELLOW);
    }
    
    const char* instrucao = "Pressione ENTER para voltar ao Menu";
    int larguraInstrucao = MeasureText(instrucao, 18);
    DrawText(instrucao, screenWidth / 2 - larguraInstrucao / 2, screenHeight / 2 + 80, 18, LIGHTGRAY);
}

void DrawVictoryScreen(Game* game) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 200});
    
    const char* titulo = "NIVEL COMPLETO!";
    int larguraTitulo = MeasureText(titulo, 45);
    DrawText(titulo, screenWidth / 2 - larguraTitulo / 2, screenHeight / 2 - 80, 45, GREEN);
    
    char scoreText[64];
    sprintf(scoreText, "Pontuacao: %d", game->score);
    int larguraScore = MeasureText(scoreText, 25);
    DrawText(scoreText, screenWidth / 2 - larguraScore / 2, screenHeight / 2 - 10, 25, WHITE);
    
    char levelText[64];
    sprintf(levelText, "Proximo Nivel: %d", game->currentLevel + 1);
    int larguraLevel = MeasureText(levelText, 20);
    DrawText(levelText, screenWidth / 2 - larguraLevel / 2, screenHeight / 2 + 30, 20, YELLOW);
    
    const char* aguarde = "Aguarde...";
    int larguraAguarde = MeasureText(aguarde, 18);
    DrawText(aguarde, screenWidth / 2 - larguraAguarde / 2, screenHeight / 2 + 70, 18, LIGHTGRAY);
}

void DrawLevelTransition(Game* game, GameState* state) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    ClearBackground(BLACK);
    
    char levelText[64];
    sprintf(levelText, "NIVEL %d", game->currentLevel);
    int largura = MeasureText(levelText, 50);
    DrawText(levelText, screenWidth / 2 - largura / 2, screenHeight / 2 - 25, 50, YELLOW);
}

void DrawHUD(Game* game, GameState* state) {
    int screenWidth = GetScreenWidth();
    int hudY = LINHAS * TAMANHO_BLOCO + 5;
    
    char scoreText[32];
    sprintf(scoreText, "Score: %d", game->score);
    DrawText(scoreText, 10, hudY, 20, WHITE);
    
    char livesText[32];
    sprintf(livesText, "Vidas: %d", game->lives);
    DrawText(livesText, 200, hudY, 20, RED);
    
    char levelText[32];
    sprintf(levelText, "Nivel: %d", game->currentLevel);
    DrawText(levelText, 380, hudY, 20, YELLOW);
    
    if (game->mapa != NULL) {
        char pelletsText[32];
        sprintf(pelletsText, "Pellets: %d", game->mapa->numPellets);
        DrawText(pelletsText, 560, hudY, 20, LIGHTGRAY);
    }
    
    if (state->isInvulnerable) {
        DrawText("INVULNERAVEL!", screenWidth / 2 - 70, 10, 20, SKYBLUE);
    }
}