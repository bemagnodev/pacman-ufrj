#ifndef GHOST_H
#define GHOST_H

#include "game.h"

void InitGhosts(Game* game);

void UpdateGhosts(Game* game, float delta);

void DrawGhosts(Game* game);

void UnloadGhosts(Game* game);

void ScareGhosts(Game* game);

#endif 