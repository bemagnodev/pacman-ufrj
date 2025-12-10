#include "persistence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Salva o jogo em binário
static bool SaveGame(Game* game) {
    if (game->mapa == NULL) return false;

    FILE* file = fopen(SAVEGAME_FILENAME, "wb");
    if (file == NULL) {
        printf("Erro ao abrir arquivo para salvar.\n");
        return false;
    }

    // 1. Dados Básicos
    fwrite(&game->score, sizeof(int), 1, file);
    fwrite(&game->lives, sizeof(int), 1, file);
    fwrite(&game->currentLevel, sizeof(int), 1, file);
    
    // 2. Pacman
    fwrite(&game->pacman, sizeof(Pacman), 1, file);

    // 3. Fantasmas
    fwrite(&game->ghostCount, sizeof(int), 1, file);
    fwrite(game->ghosts, sizeof(Ghost), game->ghostCount, file);

    // 4. Mapa (Dados dinâmicos)
    fwrite(&game->mapa->numPellets, sizeof(int), 1, file);
    
    for (int i = 0; i < game->mapa->linhas; i++) {
        fwrite(game->mapa->matriz[i], sizeof(Blocos), game->mapa->colunas, file);
    }

    fclose(file);
    return true;
}

// Lógica de Carregamento Segura
static bool LoadGame(Game* game) {
    FILE* file = fopen(SAVEGAME_FILENAME, "rb");
    if (file == NULL) return false; 

    int savedScore, savedLives, savedLevel;

    fread(&savedScore, sizeof(int), 1, file);
    fread(&savedLives, sizeof(int), 1, file);
    fread(&savedLevel, sizeof(int), 1, file);

    // Limpa estado anterior
    if (game->mapa != NULL) descartarMapa(game->mapa);
    if (game->ghosts != NULL) { free(game->ghosts); game->ghosts = NULL; }

    // Aloca mapa base
    char filename[30];
    sprintf(filename, "../mapa%d.txt", savedLevel);
    game->mapa = lerMapa(filename);

    if (game->mapa == NULL) {
        fclose(file);
        return false;
    }

    game->score = savedScore;
    game->lives = savedLives;
    game->currentLevel = savedLevel;

    fread(&game->pacman, sizeof(Pacman), 1, file);

    int savedGhostCount;
    fread(&savedGhostCount, sizeof(int), 1, file);
    
    game->ghostCount = savedGhostCount;
    game->ghosts = (Ghost*)malloc(sizeof(Ghost) * game->ghostCount);
    fread(game->ghosts, sizeof(Ghost), game->ghostCount, file);

    int savedPellets;
    fread(&savedPellets, sizeof(int), 1, file);
    game->mapa->numPellets = savedPellets;

    for (int i = 0; i < game->mapa->linhas; i++) {
        fread(game->mapa->matriz[i], sizeof(Blocos), game->mapa->colunas, file);
    }

    fclose(file);
    return true;
}

// === MENU VISUAL E COLORIDO ===
void DrawPauseOverlay(Game* game, bool* shouldQuit, bool* shouldStartNew) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    // 1. Fundo Escurecido
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 200});
    
    // 2. Caixa do Menu
    int boxWidth = 600;
    int boxHeight = 500;
    int boxX = screenWidth / 2 - boxWidth / 2;
    int boxY = screenHeight / 2 - boxHeight / 2;
    
    DrawRectangle(boxX, boxY, boxWidth, boxHeight, (Color){10, 10, 30, 240});
    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, YELLOW);
    DrawRectangleLines(boxX - 2, boxY - 2, boxWidth + 4, boxHeight + 4, ORANGE); 

    // 3. Título
    const char* titulo = "PAUSA - OPCOES";
    int tamanhoTitulo = 50;
    int larguraTitulo = MeasureText(titulo, tamanhoTitulo);
    DrawText(titulo, screenWidth / 2 - larguraTitulo / 2 + 4, boxY + 44, tamanhoTitulo, BLACK);
    DrawText(titulo, screenWidth / 2 - larguraTitulo / 2, boxY + 40, tamanhoTitulo, YELLOW);
    
    // 4. Opções Coloridas
    int fontSize = 30;
    int spacing = 60;
    int startOptionsY = boxY + 130;
    int textX = boxX + 150; 
    int keyX = boxX + 80;   
    
    // [N] Novo Jogo
    DrawText("[N]", keyX, startOptionsY, fontSize, SKYBLUE);
    DrawText("Novo Jogo", textX, startOptionsY, fontSize, SKYBLUE);
    
    // [C] Carregar
    DrawText("[C]", keyX, startOptionsY + spacing, fontSize, ORANGE);
    DrawText("Carregar Jogo", textX, startOptionsY + spacing, fontSize, ORANGE);

    // [S] Salvar
    DrawText("[S]", keyX, startOptionsY + spacing * 2, fontSize, MAGENTA);
    DrawText("Salvar Jogo", textX, startOptionsY + spacing * 2, fontSize, MAGENTA);

    // [V] Voltar
    DrawText("[V]", keyX, startOptionsY + spacing * 3, fontSize, GREEN);
    DrawText("Voltar ao Jogo", textX, startOptionsY + spacing * 3, fontSize, GREEN);

    // [Q] Sair
    DrawText("[Q]", keyX, startOptionsY + spacing * 4, fontSize, RED);
    DrawText("Sair do Jogo", textX, startOptionsY + spacing * 4, fontSize, RED);
    
    // 5. Rodapé Informativo
    DrawLine(boxX + 20, boxY + boxHeight - 60, boxX + boxWidth - 20, boxY + boxHeight - 60, GRAY);
    
    char infoGame[128];
    sprintf(infoGame, "Score: %06d   |   Vidas: %d   |   Nivel: %d", game->score, game->lives, game->currentLevel);
    int larguraInfo = MeasureText(infoGame, 20);
    DrawText(infoGame, screenWidth / 2 - larguraInfo / 2, boxY + boxHeight - 40, 20, LIGHTGRAY);
}

void HandlePauseMenu(Game* game, bool* shouldQuit, bool* shouldStartNew) {
    if (IsKeyPressed(KEY_N)) { 
        *shouldStartNew = true;
        game->isPaused = false;
    } 
    else if (IsKeyPressed(KEY_S)) { 
        if (SaveGame(game)) {
            printf("Jogo salvo!\n");
            game->saveMessageTimer = 2.0f;
        }
    } 
    else if (IsKeyPressed(KEY_C)) { 
        if (LoadGame(game)) {
            game->isPaused = false;
            printf("Jogo carregado!\n");
        }
    } 
    else if (IsKeyPressed(KEY_Q)) { 
        *shouldQuit = true;
    } 
    else if (IsKeyPressed(KEY_V) || IsKeyPressed(KEY_ESCAPE)) { 
        game->isPaused = false; 
    }
}

// --- Funções de Ranking ---
void LoadRanking(int scores[MAX_RANKING_SCORES]) {
    for (int i = 0; i < MAX_RANKING_SCORES; i++) scores[i] = 0;
    FILE* file = fopen(RANKING_FILENAME, "rb");
    if (file != NULL) {
        fread(scores, sizeof(int), MAX_RANKING_SCORES, file);
        fclose(file);
    }
}

void UpdateAndSaveRanking(Game* game) {
    int scores[MAX_RANKING_SCORES];
    LoadRanking(scores);

    int newRank = -1;
    for (int i = 0; i < MAX_RANKING_SCORES; i++) {
        if (game->score > scores[i]) {
            newRank = i;
            break;
        }
    }

    if (newRank != -1) {
        for (int i = MAX_RANKING_SCORES - 1; i > newRank; i--) {
            scores[i] = scores[i - 1];
        }
        scores[newRank] = game->score;
        sprintf(game->rankingMessage, "Voce obteve a %da maior pontuacao!", newRank + 1);
        FILE* file = fopen(RANKING_FILENAME, "wb");
        if (file != NULL) {
            fwrite(scores, sizeof(int), MAX_RANKING_SCORES, file);
            fclose(file);
        }
    } else {
        sprintf(game->rankingMessage, "FIM DE JOGO!");
    }
}