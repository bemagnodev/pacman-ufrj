#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "game.h"

#define SAVEGAME_FILENAME "savegame.bin"
#define RANKING_FILENAME "ranking.bin"
#define MAX_RANKING_SCORES 5  // Define que o top 5 será mostrado

// Desenha o menu de pausa (Visual)
void DrawPauseOverlay(Game* game, bool* shouldQuit, bool* shouldStartNew);

// Gerencia a lógica do menu (Teclas N, S, C, Q, V)
void HandlePauseMenu(Game* game, bool* shouldQuit, bool* shouldStartNew);

// Atualiza o ranking se o jogador bateu recorde (Chamado no Game Over)
void UpdateAndSaveRanking(Game* game);

// Lê a lista de recordes (Usado pela tela de Game Over para desenhar a tabela)
void LoadRanking(int scores[MAX_RANKING_SCORES]);

#endif // PERSISTENCE_H