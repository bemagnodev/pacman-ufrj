/*
 * ARQUIVO: persistence.h 
 *
 * DESCRIÇÃO:
 * Interface para o módulo de persistência e menus.
 * usará para gerenciar o menu de pausa, salvar, carregar
 * e atualizar o ranking.
 */

#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "game.h" // Inclui as structs compartilhadas

// Constantes
#define SAVEGAME_FILENAME "savegame.bin"
#define RANKING_FILENAME "ranking.data"
#define MAX_RANKING_SCORES 10

// Função: Desenha e gerencia a lógica do menu de pausa.
void HandlePauseMenu(Game* game, bool* shouldQuit, bool* shouldStartNew);

// Função (EXTRA): Verifica se a pontuação entra no Top 10 e salva.
void UpdateAndSaveRanking(Game* game);

// Função (EXTRA): Carrega as pontuações do ranking do arquivo.
void LoadRanking(int scores[MAX_RANKING_SCORES]);


#endif // PERSISTENCE_H