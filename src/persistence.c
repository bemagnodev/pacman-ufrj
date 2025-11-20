/*
 * ARQUIVO: persistence.c 
 *
 * FOCO: Menus, Persistência de Dados e Ranking.
 *
 * TAREFAS:
 * - Implementar menu de pausa (TAB).
 * - Funcionalidade de Salvar Jogo (binário).
 * - Funcionalidade de Carregar Jogo (binário).
 * - Lógica de "Novo Jogo" e "Sair".
 * - [EXTRA] Sistema de Ranking.
 */

#include "persistence.h"
#include <stdio.h>  // Para operações com arquivos (FILE)
#include <stdlib.h> // Para alocação dinâmica (malloc/free)
#include <string.h> // Para formatar strings (sprintf)

// Lógica interna para salvar o jogo
static bool SaveGame(Game* game) {
    // Abre (ou cria) o arquivo "savegame.bin" em modo "write binary" (wb)
    FILE* file = fopen(SAVEGAME_FILENAME, "wb");
    if (file == NULL) {
        return false; // Retorna falha se não conseguir abrir o arquivo
    }

    // Escreve os dados simples (int, bool) direto da struct 'game' para o arquivo
    fwrite(&game->score, sizeof(int), 1, file);
    fwrite(&game->lives, sizeof(int), 1, file);
    fwrite(&game->level, sizeof(int), 1, file);
    fwrite(&game->pelletsRemaining, sizeof(int), 1, file);

    // Escreve a struct inteira do Pac-Man para o arquivo
    fwrite(&game->pacman, sizeof(Pacman), 1, file);

    // Salva o número de fantasmas
    fwrite(&game->ghostCount, sizeof(int), 1, file);
    // Salva o array de structs de fantasmas (o conteúdo do ponteiro)
    fwrite(game->ghosts, sizeof(Ghost), game->ghostCount, file);

    // Salva o mapa (matriz 2D dinâmica)
    // Itera por cada linha do mapa
    for (int i = 0; i < MAP_ROWS; i++) {
        // Escreve o array de chars (a linha inteira) no arquivo
        fwrite(game->map[i], sizeof(char), MAP_COLS, file);
    }

    // Fecha o arquivo e retorna sucesso
    fclose(file);
    return true;
}

// Lógica interna para carregar o jogo
static bool LoadGame(Game* game) {
    // Abre o arquivo "savegame.bin" em modo "read binary" (rb)
    FILE* file = fopen(SAVEGAME_FILENAME, "rb");
    if (file == NULL) {
        return false; // Retorna falha se o arquivo não existir
    }

    // Lê os dados simples do arquivo e armazena nos ponteiros
    fread(&game->score, sizeof(int), 1, file);
    fread(&game->lives, sizeof(int), 1, file);
    fread(&game->level, sizeof(int), 1, file);
    fread(&game->pelletsRemaining, sizeof(int), 1, file);

    // Lê a struct do Pac-Man do arquivo
    fread(&game->pacman, sizeof(Pacman), 1, file);

    // Carrega o número de fantasmas
    int newGhostCount = 0;
    fread(&newGhostCount, sizeof(int), 1, file); // Lê a contagem salva

    // Se a contagem for diferente, realoca o vetor
    // (Coordenação com Membro 3)
    if (newGhostCount != game->ghostCount) {
        free(game->ghosts); // Libera o vetor antigo
        game->ghosts = (Ghost*)malloc(sizeof(Ghost) * newGhostCount);
        game->ghostCount = newGhostCount;
    }
    // Lê os dados dos fantasmas do arquivo para o vetor
    fread(game->ghosts, sizeof(Ghost), game->ghostCount, file);

    // Carrega o mapa (matriz 2D dinâmica)
    // (Coordenação com Membro 1: o mapa já deve estar alocado)
    for (int i = 0; i < MAP_ROWS; i++) {
        // Lê uma linha do arquivo e coloca no mapa do jogo
        fread(game->map[i], sizeof(char), MAP_COLS, file);
    }

    // Fecha o arquivo e retorna sucesso
    fclose(file);
    return true;
}

// Função principal do menu, chamada pelo Membro 4
void HandlePauseMenu(Game* game, bool* shouldQuit, bool* shouldStartNew) {
    
    // --- Lógica de Input do Menu ---
    
    if (IsKeyPressed(KEY_N)) { // Tecla 'N' (Novo Jogo)
        *shouldStartNew = true; // Avisa ao main que deve reiniciar o jogo
        game->isPaused = false;   // Sai do menu
    } 
    else if (IsKeyPressed(KEY_S)) { // Tecla 'S' (Salvar)
        SaveGame(game); // Chama a função interna de salvar
        // O jogo continua pausado
    } 
    else if (IsKeyPressed(KEY_C)) { // Tecla 'C' (Carregar)
        if (LoadGame(game)) { // Tenta carregar o jogo
            game->isPaused = false; // Se conseguir, sai do menu
        }
    } 
    else if (IsKeyPressed(KEY_Q)) { // Tecla 'Q' (Sair)
        *shouldQuit = true; // Avisa ao main que deve fechar o programa
    } 
    else if (IsKeyPressed(KEY_V) || IsKeyPressed(KEY_TAB)) { // Tecla 'V' ou 'TAB' (Voltar)
        game->isPaused = false; // Apenas sai do menu
    }

    // --- Desenho do Menu ---
    
    // Desenha um retângulo preto semi-transparente sobre a tela
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 0, 0, 150});
    
    // Pega o tamanho da tela para centralizar o texto
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Desenha as opções do menu
    DrawText("JOGO PAUSADO", screenWidth / 2 - MeasureText("JOGO PAUSADO", 40) / 2, screenHeight / 2 - 100, 40, YELLOW);
    DrawText("[N] Novo Jogo", screenWidth / 2 - 100, screenHeight / 2 - 20, 20, WHITE);
    DrawText("[S] Salvar Jogo", screenWidth / 2 - 100, screenHeight / 2 + 10, 20, WHITE);
    DrawText("[C] Carregar Jogo", screenWidth / 2 - 100, screenHeight / 2 + 40, 20, WHITE);
    DrawText("[V] Voltar", screenWidth / 2 - 100, screenHeight / 2 + 70, 20, LIME);
    DrawText("[Q] Sair do Jogo", screenWidth / 2 - 100, screenHeight / 2 + 100, 20, RED);
}


// --- Funções do [EXTRA] Ranking ---

// Carrega o ranking do arquivo para um array
void LoadRanking(int scores[MAX_RANKING_SCORES]) {
    // Zera o array de scores (caso o arquivo não exista)
    for (int i = 0; i < MAX_RANKING_SCORES; i++) {
        scores[i] = 0;
    }

    // Tenta abrir o arquivo de ranking
    FILE* file = fopen(RANKING_FILENAME, "rb");
    if (file != NULL) {
        // Se existir, lê os scores para o array
        fread(scores, sizeof(int), MAX_RANKING_SCORES, file);
        fclose(file);
    }
}

// Atualiza o ranking com a pontuação final e salva
void UpdateAndSaveRanking(Game* game) {
    int scores[MAX_RANKING_SCORES];
    LoadRanking(scores); // Carrega os scores antigos

    // Encontra a posição da nova pontuação
    int newRank = -1; // -1 = não entrou no ranking
    for (int i = 0; i < MAX_RANKING_SCORES; i++) {
        if (game->score > scores[i]) {
            newRank = i; // Encontra a posição (ex: 0 = 1º lugar)
            break; // Para na primeira posição que a pontuação é maior
        }
    }

    // Se o jogador entrou no ranking (newRank != -1)
    if (newRank != -1) {
        // Desloca as pontuações antigas para abrir espaço
        for (int i = MAX_RANKING_SCORES - 1; i > newRank; i--) {
            scores[i] = scores[i - 1];
        }

        // Insere a nova pontuação na posição correta
        scores[newRank] = game->score;

        // Cria a mensagem de parabéns
        sprintf(game->rankingMessage, "Voce obteve a %da maior pontuacao!", newRank + 1);

        // Salva o novo ranking (atualizado) no arquivo
        FILE* file = fopen(RANKING_FILENAME, "wb");
        if (file != NULL) {
            fwrite(scores, sizeof(int), MAX_RANKING_SCORES, file);
            fclose(file);
        }
    } else {
        // Se não entrou no ranking, apenas define a mensagem de "Fim de Jogo"
        sprintf(game->rankingMessage, "FIM DE JOGO!");
    }
}