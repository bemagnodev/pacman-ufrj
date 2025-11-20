#include "ghost.h"
#include <stdlib.h>
#include <math.h>
#include <float.h> 

bool IsValidTile(Game* game, int gridX, int gridY) {
    if (gridX < 0 || gridX >= game->mapa->colunas) return false;
    if (gridY < 0 || gridY >= game->mapa->linhas) return false;
    if (game->mapa->matriz[gridY][gridX] == PAREDE) return false;
    return true;
}

void InitGhosts(Game* game) {
    if (game->mapa == NULL) return;

    game->ghostCount = game->mapa->numFantasmas;
    game->ghosts = (Ghost*) malloc(sizeof(Ghost) * game->ghostCount);

    Color cores[4] = { RED, PINK, SKYBLUE, ORANGE };
    Direcao dirs[4] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };

    for (int i = 0; i < game->ghostCount; i++) {
        int gridX = game->mapa->fantasmas_inicio[i].x;
        int gridY = game->mapa->fantasmas_inicio[i].y;

        game->ghosts[i].position = (Vector2){ (float)(gridX * TAMANHO_BLOCO), (float)(gridY * TAMANHO_BLOCO) };
        game->ghosts[i].originalColor = cores[i % 4];
        game->ghosts[i].color = game->ghosts[i].originalColor;
        
        int r = GetRandomValue(0, 3);
        game->ghosts[i].direction = dirs[r]; 
        game->ghosts[i].podeDecidir = true;

        game->ghosts[i].isVulnerable = false;
        game->ghosts[i].vulnerableTimer = 0.0f;
        
        game->ghosts[i].isActive = true; 
    }
}

void ScareGhosts(Game* game) {
    for (int i = 0; i < game->ghostCount; i++) {
        if (!game->ghosts[i].isActive) continue; 

        game->ghosts[i].isVulnerable = true;
        game->ghosts[i].vulnerableTimer = 8.0f;
        game->ghosts[i].color = RAYWHITE;
    }
}



float CalcularDistancia(int x1, int y1, int x2, int y2) {
    return (float)((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}

void UpdateGhosts(Game* game, float delta) {
    float margemCentro = 2.0f;
    
    Vector2 mousePos = GetMousePosition();
    int alvoX = (int)(mousePos.x / TAMANHO_BLOCO);
    int alvoY = (int)(mousePos.y / TAMANHO_BLOCO);

    for (int i = 0; i < game->ghostCount; i++) {
        Ghost* g = &game->ghosts[i];

        if (!g->isActive) continue;

        float velocidadeAtual = g->isVulnerable ? 3.0f * TAMANHO_BLOCO : 4.0f * TAMANHO_BLOCO;
        if(g->isVulnerable) {
            g->vulnerableTimer -= delta;
            if(g->vulnerableTimer <= 0) { g->isVulnerable = false; g->color = g->originalColor; }
        }

        g->position.x += g->direction.x * velocidadeAtual * delta;
        g->position.y += g->direction.y * velocidadeAtual * delta;

        int gridX = (int)((g->position.x + TAMANHO_BLOCO / 2) / TAMANHO_BLOCO);
        int gridY = (int)((g->position.y + TAMANHO_BLOCO / 2) / TAMANHO_BLOCO);
        float centroX = gridX * TAMANHO_BLOCO;
        float centroY = gridY * TAMANHO_BLOCO;
        float dist = fabs(g->position.x - centroX) + fabs(g->position.y - centroY);

        if (dist < margemCentro) {
            if (g->podeDecidir) {
                g->position.x = centroX;
                g->position.y = centroY;

                Direcao possiveis[4];
                int qtdPossiveis = 0;
                Direcao dirs[4] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };

                for (int d = 0; d < 4; d++) {
                    if (dirs[d].x == -g->direction.x && dirs[d].y == -g->direction.y) continue;
                    if (IsValidTile(game, gridX + dirs[d].x, gridY + dirs[d].y)) {
                        possiveis[qtdPossiveis] = dirs[d];
                        qtdPossiveis++;
                    }
                }

                bool bateuFrente = !IsValidTile(game, gridX + g->direction.x, gridY + g->direction.y);

                if (qtdPossiveis > 1 || bateuFrente || qtdPossiveis == 0) {
                    if (qtdPossiveis > 0) {
                        
                        if ((i == 0 || i == 1) && !g->isVulnerable) {
                            float menorDistancia = FLT_MAX;
                            int melhorIndice = 0;

                            for (int k = 0; k < qtdPossiveis; k++) {
                                int nextX = gridX + possiveis[k].x;
                                int nextY = gridY + possiveis[k].y;
                                
                                float d = CalcularDistancia(nextX, nextY, alvoX, alvoY);
                                if (d < menorDistancia) {
                                    menorDistancia = d;
                                    melhorIndice = k;
                                }
                            }
                            g->direction = possiveis[melhorIndice];

                        } else {
                            int r = GetRandomValue(0, qtdPossiveis - 1);
                            g->direction = possiveis[r];
                        }

                    } else {
                        g->direction.x = -g->direction.x;
                        g->direction.y = -g->direction.y;
                    }
                }
                g->podeDecidir = false;
            }
        } else {
            g->podeDecidir = true;
        }
    }
}

void DrawGhosts(Game* game) {
    int offset = TAMANHO_BLOCO / 2;
    int raio = (TAMANHO_BLOCO / 2) - 2;

    for (int i = 0; i < game->ghostCount; i++) {
        if (!game->ghosts[i].isActive) continue; 

        DrawCircle((int)game->ghosts[i].position.x + offset, (int)game->ghosts[i].position.y + offset, raio, game->ghosts[i].color);
    }
}

void UnloadGhosts(Game* game) {
    if (game->ghosts != NULL) {
        free(game->ghosts);
        game->ghosts = NULL;
    }
}