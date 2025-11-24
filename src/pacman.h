/*
 * ARQUIVO: pacman.h
 *
 * DESCRIÇÃO:
 * Interface para movimentação e desenho do Pac-Man
 */

#ifndef PACMAN_H
#define PACMAN_H

#include "game.h"

// Constantes de movimentação
#define PACMAN_SPEED 4.1f  // Velocidade do Pac-Man (blocos por segundo)

// Inicializa o Pac-Man
void InitPacman(Game* game);

// Atualiza a movimentação do Pac-Man
void UpdatePacman(Game* game, float delta);

// Desenha o Pac-Man
void DrawPacman(Game* game);

// Verifica se uma posição é válida (não é parede)
bool IsPacmanValidTile(Game* game, int gridX, int gridY);

#endif // PACMAN_H