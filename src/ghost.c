#include "ghost.h"
#include <stdlib.h>
#include <math.h>
#include <float.h> 

// Verifica se uma posição esta dentro dos limites e não é paprede
bool IsValidTile(Game* game, int gridX, int gridY) {
    if (gridX < 0 || gridX >= game->mapa->colunas) return false;
    if (gridY < 0 || gridY >= game->mapa->linhas) return false;
    if (game->mapa->matriz[gridY][gridX] == PAREDE) return false;
    return true;
}

// Verifica colisão entre fantasmas para evitar sobreposição
bool IsGhostAtTile(Game* game, int gridX, int gridY, int ignoreIndex) {
    for (int i = 0; i < game->ghostCount; i++) {
        if (i == ignoreIndex) continue; // Não verifica a si mesmo
        if (!game->ghosts[i].isActive) continue;

        // Converte a posição de pixel do outro fantasma para Grid
        int otherGridX = (int)((game->ghosts[i].position.x + TAMANHO_BLOCO / 2) / TAMANHO_BLOCO);
        int otherGridY = (int)((game->ghosts[i].position.y + TAMANHO_BLOCO / 2) / TAMANHO_BLOCO);

        if (otherGridX == gridX && otherGridY == gridY) {
            return true; // Tem fantasma aqui
        }
    }
    return false;
}

// Inicializa o vetor de fantasmas via Alocação Dinâmica, definindo (posição, cor e Direção)
void InitGhosts(Game* game) {
    if (game->mapa == NULL) return;

    game->ghostCount = game->mapa->numFantasmas;
    game->ghosts = (Ghost*) malloc(sizeof(Ghost) * game->ghostCount);

    Color cores[4] = { RED, PINK, SKYBLUE, ORANGE };
    Direcao dirs[4] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };

    for (int i = 0; i < game->ghostCount; i++) {
        int gridX = game->mapa->fantasmas_inicio[i].x;
        int gridY = game->mapa->fantasmas_inicio[i].y;

        // Define posição inicial em pixels
        game->ghosts[i].position = (Vector2){ (float)(gridX * TAMANHO_BLOCO), (float)(gridY * TAMANHO_BLOCO) };
        // Cores cíclicas (Vermelho, Rosa, Ciano, Laranja)
        game->ghosts[i].originalColor = cores[i % 4];
        game->ghosts[i].color = game->ghosts[i].originalColor;
        
        // Direção inicial aleatória
        int r = GetRandomValue(0, 3);
        game->ghosts[i].direction = dirs[r]; 
        game->ghosts[i].podeDecidir = true;

        // Reset de estados
        game->ghosts[i].isVulnerable = false;
        game->ghosts[i].vulnerableTimer = 0.0f;
        game->ghosts[i].isActive = true; 
    }
}

// Ativa o modo vulnerável - power pellet
void ScareGhosts(Game* game) {
    for (int i = 0; i < game->ghostCount; i++) {
        if (!game->ghosts[i].isActive) continue; 

        if (!game->ghosts[i].isVulnerable) {
            game->ghosts[i].direction.x *= -1; // Inverte direção X
            game->ghosts[i].direction.y *= -1; // Inverte direção Y
        }

        game->ghosts[i].isVulnerable = true;
        game->ghosts[i].vulnerableTimer = 8.0f; 
        
    }
}

// Caucula distancia euclidiana  
float CalcularDistancia(int x1, int y1, int x2, int y2) {
    return (float)((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}

// Loop principal de lógica dos fantasmas (60x/s)
void UpdateGhosts(Game* game, float delta) {
    float margemCentro = 2.0f;
    
    int alvoX = game->pacman.gridPos.x;
    int alvoY = game->pacman.gridPos.y;

    for (int i = 0; i < game->ghostCount; i++) {
        Ghost* g = &game->ghosts[i];

        if (!g->isActive) continue;

        // Velocidade: 3.0f quando vulnerável, 4.0f normal
        float velocidadeAtual = g->isVulnerable ? 3.0f * TAMANHO_BLOCO : 4.0f * TAMANHO_BLOCO;
        
        if(g->isVulnerable) {
            g->vulnerableTimer -= delta;
            if(g->vulnerableTimer <= 0) { 
                g->isVulnerable = false; 
                g->color = g->originalColor; 
            }
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
                    
                    int nextX = gridX + dirs[d].x;
                    int nextY = gridY + dirs[d].y;

                    // Colisão com Parede e Outros Fantasmas
                    bool ehParede = !IsValidTile(game, nextX, nextY);
                    bool temFantasma = IsGhostAtTile(game, nextX, nextY, i);

                    if (!ehParede && !temFantasma) {
                        possiveis[qtdPossiveis] = dirs[d];
                        qtdPossiveis++;
                    }
                }

                int frontX = gridX + g->direction.x;
                int frontY = gridY + g->direction.y;
                bool bateuFrente = !IsValidTile(game, frontX, frontY) || IsGhostAtTile(game, frontX, frontY, i);

                if (qtdPossiveis > 1 || bateuFrente || qtdPossiveis == 0) {
                    if (qtdPossiveis > 0) {
                        // Inteligência do Fantasma 0 (Blinky)
                        if (i == 0 && !g->isVulnerable) {
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
                            // Aleatório para os outros
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

        // Lógica de Portal
        int currentGridX = (int)((g->position.x + TAMANHO_BLOCO / 2) / TAMANHO_BLOCO);
        int currentGridY = (int)((g->position.y + TAMANHO_BLOCO / 2) / TAMANHO_BLOCO);

        if (game->mapa->matriz[currentGridY][currentGridX] == PORTAL) {
             if (game->mapa->numPortais >= 2) {
                bool entering = false;
                if (currentGridX == 0 && g->direction.x < 0) entering = true;
                else if (currentGridX == game->mapa->colunas - 1 && g->direction.x > 0) entering = true;
                
                if (entering) {
                    int idx = -1;
                    for(int k=0; k<game->mapa->numPortais; k++) {
                        if (game->mapa->portais[k].x == currentGridX && game->mapa->portais[k].y == currentGridY) {
                            idx = k; break;
                        }
                    }
                    if (idx != -1) {
                        int dest = (idx + 1) % game->mapa->numPortais;
                        g->position.x = game->mapa->portais[dest].x * TAMANHO_BLOCO;
                        g->position.y = game->mapa->portais[dest].y * TAMANHO_BLOCO;
                    }
                }
             }
        }
    } 
}

// Desenha fantasmas
void DrawGhosts(Game* game) {
    int offset = TAMANHO_BLOCO / 2;
    int raioCabeca = (TAMANHO_BLOCO / 2) - 2; 

    for (int i = 0; i < game->ghostCount; i++) {
        if (!game->ghosts[i].isActive) continue; 
        
        int gx = (int)game->ghosts[i].position.x + offset;
        int gy = (int)game->ghosts[i].position.y + offset;
        Color corFantasma = game->ghosts[i].color;

        // Fica AZUL quando vulnerável  
        if (game->ghosts[i].isVulnerable) {
            if (game->ghosts[i].vulnerableTimer < 2.0f && (int)(GetTime()*10)%2==0) {
                corFantasma = RAYWHITE; 
            } else {
                corFantasma = BLUE; 
            }
        }

        // 1. Cabeça
        DrawCircle(gx, gy - 2, raioCabeca, corFantasma);
        // 2. Corpo
        DrawRectangle(gx - raioCabeca, gy - 2, raioCabeca * 2, raioCabeca, corFantasma);
        
        // 3. Pés
        int peRaio = raioCabeca / 3;
        DrawCircle(gx - raioCabeca + peRaio, gy + raioCabeca - 2, peRaio, corFantasma);
        DrawCircle(gx, gy + raioCabeca - 2, peRaio, corFantasma);
        DrawCircle(gx + raioCabeca - peRaio, gy + raioCabeca - 2, peRaio, corFantasma);

        // 4. Rosto  
        if (!game->ghosts[i].isVulnerable) {
            int olhoOffsetX = 4;
            if(TAMANHO_BLOCO > 30) olhoOffsetX = 8;
            int olhoOffsetY = -2;
            int olhoRaio = raioCabeca / 3;
            
            DrawCircle(gx - olhoOffsetX, gy + olhoOffsetY, olhoRaio, WHITE);
            DrawCircle(gx + olhoOffsetX, gy + olhoOffsetY, olhoRaio, WHITE);

            int pupilaOffsetX = game->ghosts[i].direction.x * 2;
            int pupilaOffsetY = game->ghosts[i].direction.y * 2;
            int pupilaRaio = olhoRaio / 2;

            DrawCircle(gx - olhoOffsetX + pupilaOffsetX, gy + olhoOffsetY + pupilaOffsetY, pupilaRaio, DARKBLUE);
            DrawCircle(gx + olhoOffsetX + pupilaOffsetX, gy + olhoOffsetY + pupilaOffsetY, pupilaRaio, DARKBLUE);
        } else {
            // MODO VULNERÁVEL
            DrawCircle(gx - 4, gy, 2, WHITE); // Olho esq pequeno
            DrawCircle(gx + 4, gy, 2, WHITE); // Olho dir pequeno
            
            // Boca tremida  
            int bocaW = raioCabeca;
            DrawLine(gx - bocaW/2, gy + 5, gx - bocaW/4, gy + 3, WHITE);
            DrawLine(gx - bocaW/4, gy + 3, gx + bocaW/4, gy + 5, WHITE);
            DrawLine(gx + bocaW/4, gy + 5, gx + bocaW/2, gy + 3, WHITE);
        }
    }
}

//evita vazamento do memoria
void UnloadGhosts(Game* game) {
    if (game->ghosts != NULL) {
        free(game->ghosts);// Libera a memória do malloc
        game->ghosts = NULL;
    }
}