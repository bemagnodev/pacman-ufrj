#include "persistence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool SaveGame(Game* game) {
    FILE* file = fopen(SAVEGAME_FILENAME, "wb");
    if (file == NULL) {
        return false;
    }

    fwrite(&game->score, sizeof(int), 1, file);
    fwrite(&game->lives, sizeof(int), 1, file);
    fwrite(&game->currentLevel, sizeof(int), 1, file);
    
    if (game->mapa != NULL) {
        fwrite(&game->mapa->numPellets, sizeof(int), 1, file);
    } else {
        int zero = 0;
        fwrite(&zero, sizeof(int), 1, file);
    }

    fwrite(&game->pacman, sizeof(Pacman), 1, file);
    fwrite(&game->ghostCount, sizeof(int), 1, file);
    fwrite(game->ghosts, sizeof(Ghost), game->ghostCount, file);

    if (game->mapa != NULL) {
        for (int i = 0; i < game->mapa->linhas; i++) {
            fwrite(game->mapa->matriz[i], sizeof(Blocos), game->mapa->colunas, file);
        }
    }

    fclose(file);
    return true;
}

// Lógica interna para carregar o jogo (CÓDIGO ORIGINAL MANTIDO)
static bool LoadGame(Game* game) {
    FILE* file = fopen(SAVEGAME_FILENAME, "rb");
    if (file == NULL) {
        return false;
    }

    fread(&game->score, sizeof(int), 1, file);
    fread(&game->lives, sizeof(int), 1, file);
    fread(&game->currentLevel, sizeof(int), 1, file);
    
    int savedPellets;
    fread(&savedPellets, sizeof(int), 1, file);

    fread(&game->pacman, sizeof(Pacman), 1, file);

    int newGhostCount = 0;
    fread(&newGhostCount, sizeof(int), 1, file);

    if (newGhostCount != game->ghostCount) {
        free(game->ghosts);
        game->ghosts = (Ghost*)malloc(sizeof(Ghost) * newGhostCount);
        game->ghostCount = newGhostCount;
    }
    fread(game->ghosts, sizeof(Ghost), game->ghostCount, file);

    if (game->mapa != NULL) {
        for (int i = 0; i < game->mapa->linhas; i++) {
            fread(game->mapa->matriz[i], sizeof(Blocos), game->mapa->colunas, file);
        }
        game->mapa->numPellets = savedPellets;
    }

    fclose(file);
    return true;
}

// Função separada apenas para DESENHAR (CÓDIGO ORIGINAL MANTIDO)
void DrawPauseOverlay(Game* game, bool* shouldQuit, bool* shouldStartNew) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    DrawRectangle(0, 0, screenWidth, screenHeight, (Color){0, 0, 0, 200});
    
    const char* titulo = "PAUSE";
    int tamanhoTitulo = 80;
    int larguraTitulo = MeasureText(titulo, tamanhoTitulo);
    int posYTitulo = screenHeight / 2 - 200;
    
    DrawText(titulo, (screenWidth / 2 - larguraTitulo / 2) + 4, posYTitulo + 4, tamanhoTitulo, (Color){0, 0, 0, 150});
    DrawText(titulo, screenWidth / 2 - larguraTitulo / 2, posYTitulo, tamanhoTitulo, YELLOW);
    
    int boxWidth = 400;
    int boxHeight = 300;
    int boxX = screenWidth / 2 - boxWidth / 2;
    int boxY = screenHeight / 2 - 80;
    
    DrawRectangle(boxX - 4, boxY - 4, boxWidth + 8, boxHeight + 8, (Color){255, 255, 255, 30});
    DrawRectangle(boxX, boxY, boxWidth, boxHeight, (Color){20, 20, 40, 230});
    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, (Color){255, 215, 0, 180});
    
    int startY = boxY + 30;
    int spacing = 45;
    int opcaoX = boxX + 30;
    
    int yPos = startY;
    DrawRectangle(opcaoX - 5, yPos - 5, 80, 35, (Color){255, 255, 255, 50});
    DrawRectangle(opcaoX, yPos, 70, 25, (Color){255, 255, 255, 100});
    DrawText("[N]", opcaoX + 10, yPos + 3, 20, WHITE);
    DrawText("Novo Jogo", opcaoX + 90, yPos + 3, 20, WHITE);
    
    yPos = startY + spacing;
    DrawRectangle(opcaoX - 5, yPos - 5, 80, 35, (Color){100, 200, 255, 50});
    DrawRectangle(opcaoX, yPos, 70, 25, (Color){100, 200, 255, 100});
    DrawText("[S]", opcaoX + 10, yPos + 3, 20, WHITE);
    DrawText("Salvar Jogo", opcaoX + 90, yPos + 3, 20, (Color){100, 200, 255, 255});
    
    yPos = startY + (spacing * 2);
    DrawRectangle(opcaoX - 5, yPos - 5, 80, 35, (Color){255, 200, 100, 50});
    DrawRectangle(opcaoX, yPos, 70, 25, (Color){255, 200, 100, 100});
    DrawText("[C]", opcaoX + 10, yPos + 3, 20, WHITE);
    DrawText("Carregar Jogo", opcaoX + 90, yPos + 3, 20, (Color){255, 200, 100, 255});
    
    yPos = startY + (spacing * 3);
    DrawRectangle(opcaoX - 5, yPos - 5, 80, 35, (Color){0, 255, 0, 50});
    DrawRectangle(opcaoX, yPos, 70, 25, (Color){0, 255, 0, 100});
    DrawText("[ESC]", opcaoX + 3, yPos + 3, 20, WHITE);
    DrawText("Voltar ao Jogo", opcaoX + 90, yPos + 3, 20, GREEN);
    
    yPos = startY + (spacing * 4);
    DrawRectangle(opcaoX - 5, yPos - 5, 80, 35, (Color){255, 0, 0, 50});
    DrawRectangle(opcaoX, yPos, 70, 25, (Color){255, 0, 0, 100});
    DrawText("[Q]", opcaoX + 10, yPos + 3, 20, WHITE);
    DrawText("Sair do Jogo", opcaoX + 90, yPos + 3, 20, RED);
    
    const char* dica = "Pressione TAB para despausar rapidamente";
    int larguraDica = MeasureText(dica, 14);
    DrawText(dica, screenWidth / 2 - larguraDica / 2, boxY + boxHeight + 20, 14, (Color){200, 200, 200, 200});
    
    char infoScore[64];
    sprintf(infoScore, "Pontuacao Atual: %d", game->score);
    DrawText(infoScore, boxX + 20, boxY + boxHeight - 35, 16, YELLOW);
    
    char infoVidas[64];
    sprintf(infoVidas, "Vidas: %d | Nivel: %d", game->lives, game->currentLevel);
    DrawText(infoVidas, boxX + 20, boxY + boxHeight - 15, 16, LIGHTGRAY);
}

// Função principal de gerenciamento do menu
void HandlePauseMenu(Game* game, bool* shouldQuit, bool* shouldStartNew) {
    
    if (IsKeyPressed(KEY_N)) { 
        *shouldStartNew = true;
        game->isPaused = false;
    } 
    else if (IsKeyPressed(KEY_S)) { 
        if (SaveGame(game)) {
            printf("Jogo salvo com sucesso!\n");
            game->saveMessageTimer = 2.0f;
        }
    } 
    else if (IsKeyPressed(KEY_C)) { 
        if (LoadGame(game)) {
            game->isPaused = false;
            printf("Jogo carregado com sucesso!\n");
        } else {
            printf("Nenhum save encontrado!\n");
        }
    } 
    else if (IsKeyPressed(KEY_Q)) { 
        *shouldQuit = true;
    } 
    // --- ALTERAÇÃO AQUI: REMOVIDO O KEY_TAB PARA NÃO CONFLITAR COM A MAIN ---
    else if (IsKeyPressed(KEY_V) || IsKeyPressed(KEY_ESCAPE)) { 
        game->isPaused = false; 
    }
}

// --- Funções do Ranking (CÓDIGO ORIGINAL MANTIDO) ---

void LoadRanking(int scores[MAX_RANKING_SCORES]) {
    for (int i = 0; i < MAX_RANKING_SCORES; i++) {
        scores[i] = 0;
    }

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