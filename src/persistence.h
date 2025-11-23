/*
 * ARQUIVO: persistence.h (ATUALIZADO)
 *
 * ADICIONADO: DrawPauseOverlay separada
 */

#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "game.h"

// Constantes
#define SAVEGAME_FILENAME "savegame.bin"
#define RANKING_FILENAME "ranking.data"
#define MAX_RANKING_SCORES 10

// Função: Gerencia a lógica do menu de pausa (INPUT)
void HandlePauseMenu(Game* game, bool* shouldQuit, bool* shouldStartNew);

// Função: Desenha o overlay do menu de pausa (VISUAL)
void DrawPauseOverlay(Game* game, bool* shouldQuit, bool* shouldStartNew);

// Função (EXTRA): Verifica se a pontuação entra no Top 10 e salva
void UpdateAndSaveRanking(Game* game);

// Função (EXTRA): Carrega as pontuações do ranking do arquivo
void LoadRanking(int scores[MAX_RANKING_SCORES]);

#endif // PERSISTENCE_H