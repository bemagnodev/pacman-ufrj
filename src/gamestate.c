#include "gamestate.h"
#include "ghost.h"
#include "persistence.h"
#include "pacman.h"
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
    // game->isMuted é inicializado na main.c
}

void StartNewGame(Game* game, GameState* state) {
    StopSound(game->sfxIntro);
    
    if (game->mapa != NULL) descartarMapa(game->mapa);
    UnloadGhosts(game);
    
    game->score = 0;
    game->lives = INITIAL_LIVES;
    game->currentLevel = 1; 
    game->gameOver = false;
    game->isPaused = false;
    
    state->invulnerabilityTimer = 0.0f;
    state->isInvulnerable = false;
    
    char filename[30];
    sprintf(filename, "../mapa%d.txt", game->currentLevel);
    game->mapa = lerMapa(filename);  
    
    if (game->mapa != NULL) {
        InitPacman(game);
        InitGhosts(game);
        state->totalPelletsInLevel = game->mapa->numPellets;
    }
    state->currentScreen = SCREEN_PLAYING;
}

void LoadNextLevel(Game* game, GameState* state) {
    game->currentLevel++;
    if (game->currentLevel > MAX_LEVEL) game->currentLevel = 1; 
    
    if (game->mapa != NULL) descartarMapa(game->mapa);
    UnloadGhosts(game);
    
    char filename[30];
    sprintf(filename, "../mapa%d.txt", game->currentLevel);
    game->mapa = lerMapa(filename);  
    
    if (game->mapa != NULL) {
        InitPacman(game);
        InitGhosts(game);
        state->totalPelletsInLevel = game->mapa->numPellets;
    }
    
    state->invulnerabilityTimer = 0.0f;
    state->isInvulnerable = false;
    state->currentScreen = SCREEN_PLAYING;
}

void ResetPacmanPosition(Game* game) {
    if (game->mapa == NULL) return;
    InitPacman(game);
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
        
        float dx = pacmanX - ghostX;
        float dy = pacmanY - ghostY;
        float distance = sqrtf(dx * dx + dy * dy);
        
        if (distance < GHOST_COLLISION_DISTANCE) {
            if (game->ghosts[i].isVulnerable) {
                game->ghosts[i].isActive = false;
                game->score += POINTS_PER_GHOST;
                PlaySound(game->sfxEatGhost);
            } else {
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
    
    if (gridX < 0 || gridX >= game->mapa->colunas) return;
    if (gridY < 0 || gridY >= game->mapa->linhas) return;
    
    Blocos tileAtual = game->mapa->matriz[gridY][gridX];
    
    if (tileAtual == PELLET) {
        game->mapa->matriz[gridY][gridX] = VAZIO;
        game->score += POINTS_PER_PELLET;
        game->mapa->numPellets--;
        if (!IsSoundPlaying(game->sfxWaka)) PlaySound(game->sfxWaka);
    }
    else if (tileAtual == POWER_PELLET) {
        game->mapa->matriz[gridY][gridX] = VAZIO;
        game->score += POINTS_PER_POWER_PELLET;
        ScareGhosts(game);
        if (!IsSoundPlaying(game->sfxWaka)) PlaySound(game->sfxWaka);
    }
}

// === REGRAS DO JOGO ===

void HandlePacmanDeath(Game* game, GameState* state) {
    game->lives--;
    PlaySound(game->sfxDeath);
    
    game->score -= PENALTY_DEATH;
    if (game->score < 0) game->score = 0;
    
    if (game->lives <= 0) {
        game->gameOver = true;
        state->currentScreen = SCREEN_GAME_OVER;
        UpdateAndSaveRanking(game);
    } else {
        ResetPacmanPosition(game);
        state->isInvulnerable = true;
        state->invulnerabilityTimer = INVULNERABILITY_TIME;
        state->deathAnimationTimer = 0.5f;
    }
}

bool CheckVictoryCondition(Game* game, GameState* state) {
    if (game->mapa == NULL) return false;
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
    if (state->isInvulnerable) {
        state->invulnerabilityTimer -= delta;
        if (state->invulnerabilityTimer <= 0.0f) state->isInvulnerable = false;
    }
    if (state->deathAnimationTimer > 0.0f) state->deathAnimationTimer -= delta;
    if (state->transitionTimer > 0.0f) {
        state->transitionTimer -= delta;
        if (state->transitionTimer <= 0.0f && state->currentScreen == SCREEN_VICTORY) {
            LoadNextLevel(game, state);
        }
    }
    
    switch (state->currentScreen) {
        case SCREEN_PLAYING:
            CheckPacmanGhostCollision(game, state);
            CheckPelletCollection(game, state);
            if (!CheckVictoryCondition(game, state)) CheckDefeatCondition(game, state);
            break;
            
        case SCREEN_MENU:
            if (!IsSoundPlaying(game->sfxIntro)) {
                PlaySound(game->sfxIntro);
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                StartNewGame(game, state);
            }
            break;
            
        case SCREEN_GAME_OVER:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) state->currentScreen = SCREEN_MENU;
            break;
        case SCREEN_VICTORY:
            break;
        default: break;
    }
}

// === FUNÇÕES DE DESENHO ===

void DrawMenuScreen(void) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    ClearBackground((Color){10, 10, 20, 255}); 
    
    const char* titulo = "PAC-MAN";
    int tamanhoTitulo = 140; 
    int larguraTitulo = MeasureText(titulo, tamanhoTitulo);
    int tituloX = screenWidth / 2 - larguraTitulo / 2;
    int tituloY = 60;
    
    DrawText(titulo, tituloX + 8, tituloY + 8, tamanhoTitulo, DARKGRAY);
    DrawText(titulo, tituloX, tituloY, tamanhoTitulo, YELLOW);
    
    int infoY = 220;
    int spacing = 40;
    
    const char* disciplina = "Disciplina: Programacao 2 - 2025/2";
    int larguraDisc = MeasureText(disciplina, 30);
    DrawText(disciplina, screenWidth / 2 - larguraDisc / 2, infoY, 30, SKYBLUE);
    
    const char* prof = "Professor: Marcos Tomazzoli Leipnitz";
    int larguraProf = MeasureText(prof, 30);
    DrawText(prof, screenWidth / 2 - larguraProf / 2, infoY + spacing, 30, WHITE);
    
    float floatOffset = sinf(GetTime() * 3.0f) * 10.0f; 
    int decorY = screenHeight / 2 + 50 + (int)floatOffset;
    int decorCenterX = screenWidth / 2;
    
    DrawCircle(decorCenterX + 80, decorY, 40, RED);
    DrawRectangle(decorCenterX + 40, decorY, 80, 40, RED);
    DrawCircle(decorCenterX + 60, decorY - 10, 10, WHITE);
    DrawCircle(decorCenterX + 100, decorY - 10, 10, WHITE);
    DrawCircle(decorCenterX + 58, decorY - 10, 4, BLUE);
    DrawCircle(decorCenterX + 98, decorY - 10, 4, BLUE);
    
    DrawCircleSector((Vector2){decorCenterX - 80, decorY + 10}, 45, 30, 330, 20, YELLOW);
    
    const char* controles = "Setas: Mover  |  TAB: Pausa  |  F11: Tela Cheia  |  M: Mutar";
    DrawText(controles, screenWidth/2 - MeasureText(controles, 24)/2, screenHeight - 140, 24, GRAY);
    
    const char* instrucao = "Pressione [ENTER] para Iniciar";
    int tamanhoInstrucao = 40;
    int larguraInstrucao = MeasureText(instrucao, tamanhoInstrucao);
    
    if ((int)(GetTime() * 2) % 2 == 0) { 
        DrawText(instrucao, screenWidth / 2 - larguraInstrucao / 2, screenHeight - 80, tamanhoInstrucao, ORANGE);
    }
}

// === TELA DE GAME OVER COM RANKING (CORRIGIDA) ===
void DrawGameOverScreen(Game* game) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    // 1. Fundo Avermelhado
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){30, 0, 0, 230});
    
    // 2. Título
    const char* titulo = "GAME OVER";
    int tamanhoTitulo = 100;
    int larguraTitulo = MeasureText(titulo, tamanhoTitulo);
    DrawText(titulo, screenWidth / 2 - larguraTitulo / 2, 50, tamanhoTitulo, RED);
    
    // 3. Score Final
    char scoreText[64];
    sprintf(scoreText, "SEU SCORE: %06d", game->score);
    int tamanhoScore = 50;
    int larguraScore = MeasureText(scoreText, tamanhoScore);
    DrawText(scoreText, screenWidth / 2 - larguraScore / 2, 160, tamanhoScore, WHITE);
    
    // 4. Mensagem de Posição
    if (strlen(game->rankingMessage) > 0) {
        int l = MeasureText(game->rankingMessage, 30);
        DrawText(game->rankingMessage, screenWidth / 2 - l / 2, 220, 30, YELLOW);
    }

    // --- 5. TABELA DE RANKING (AQUI ESTÁ O QUE FALTAVA) ---
    int scores[MAX_RANKING_SCORES];
    LoadRanking(scores); // Carrega os dados do arquivo

    int startY = 280;
    int rankFontSize = 30;
    const char* header = "- TOP 5 MELHORES -";
    DrawText(header, screenWidth/2 - MeasureText(header, rankFontSize)/2, startY, rankFontSize, ORANGE);
    
    for (int i = 0; i < MAX_RANKING_SCORES; i++) {
        char line[64];
        // Formata: "1. 005000"
        sprintf(line, "%d. %06d", i + 1, scores[i]);
        
        int w = MeasureText(line, rankFontSize);
        int posY = startY + 40 + (i * 35);
        
        // Destaca em Verde se esse for o score atual do jogador
        Color corTexto = LIGHTGRAY;
        if (scores[i] == game->score && game->score > 0) {
            corTexto = GREEN; 
        }
        
        DrawText(line, screenWidth / 2 - w / 2, posY, rankFontSize, corTexto);
    }
    
    // 6. Rodapé
    const char* instrucao = "Pressione [ENTER] para Menu";
    if ((int)(GetTime() * 2) % 2 == 0) {
        int l = MeasureText(instrucao, 30);
        DrawText(instrucao, screenWidth / 2 - l / 2, screenHeight - 60, 30, GRAY);
    }
}

void DrawVictoryScreen(Game* game) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 200});
    
    const char* titulo = "NIVEL COMPLETO!";
    int tamanhoTitulo = 90;
    int larguraTitulo = MeasureText(titulo, tamanhoTitulo);
    DrawText(titulo, screenWidth / 2 - larguraTitulo / 2, screenHeight / 2 - 120, tamanhoTitulo, GREEN);
    
    char scoreText[64];
    sprintf(scoreText, "Pontuacao: %06d", game->score);
    int tamanhoScore = 50;
    int larguraScore = MeasureText(scoreText, tamanhoScore);
    DrawText(scoreText, screenWidth / 2 - larguraScore / 2, screenHeight / 2 - 10, tamanhoScore, WHITE);
    
    char levelText[64];
    sprintf(levelText, "Proximo Nivel: %d", game->currentLevel + 1);
    int tamanhoLevel = 40;
    int larguraLevel = MeasureText(levelText, tamanhoLevel);
    DrawText(levelText, screenWidth / 2 - larguraLevel / 2, screenHeight / 2 + 50, tamanhoLevel, YELLOW);
    
    const char* aguarde = "Aguarde...";
    int tamanhoAguarde = 36;
    int larguraAguarde = MeasureText(aguarde, tamanhoAguarde);
    DrawText(aguarde, screenWidth / 2 - larguraAguarde / 2, screenHeight / 2 + 100, tamanhoAguarde, LIGHTGRAY);
}

void DrawLevelTransition(Game* game, GameState* state) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    ClearBackground(BLACK);
    
    char levelText[64];
    sprintf(levelText, "NIVEL %d", game->currentLevel);
    int tamanho = 100;
    int largura = MeasureText(levelText, tamanho);
    DrawText(levelText, screenWidth / 2 - largura / 2, screenHeight / 2 - 50, tamanho, YELLOW);
}

void DrawHUD(Game* game, GameState* state) {
    int screenWidth = GetScreenWidth();
    int hudY = LINHAS * TAMANHO_BLOCO + 5; 
    
    int fontSize = 40;
    
    char scoreText[32];
    sprintf(scoreText, "Score: %06d", game->score);
    DrawText(scoreText, 20, hudY, fontSize, WHITE);
    
    char livesText[32];
    sprintf(livesText, "Vidas: %d", game->lives);
    DrawText(livesText, 400, hudY, fontSize, RED);
    
    char levelText[32];
    sprintf(levelText, "Nivel: %d", game->currentLevel);
    DrawText(levelText, 800, hudY, fontSize, YELLOW);
    
    if (game->mapa != NULL) {
        char pelletsText[32];
        sprintf(pelletsText, "Pellets: %d", game->mapa->numPellets);
        DrawText(pelletsText, 1100, hudY, fontSize, LIGHTGRAY);
    }
    
    if (game->saveMessageTimer > 0) {
        DrawText("JOGO SALVO!", screenWidth/2 - 120, 20, fontSize, GREEN); 
    }
    else if (state->isInvulnerable) {
        DrawText("INVULNERAVEL!", screenWidth/2 - 150, 20, fontSize, SKYBLUE);
    }
}