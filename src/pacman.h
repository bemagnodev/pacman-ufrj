#ifndef PACMAN_H
#define PACMAN_H

#include "game.h"

// Inicializa o Pacman (posição, vidas, score, direção)
void InitPacman(Game* game);

// Atualiza movimento, input, colisões com mapa e fantasmas
void UpdatePacman(Game* game, float delta);

// Desenha o Pacman na tela
void DrawPacman(Game* game);

#endif // PACMAN_H