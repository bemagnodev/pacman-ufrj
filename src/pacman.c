#include "pacman.h"
#include "ghost.h" // Necessário para chamar ScareGhosts e checar colisão
#include <math.h>

// Função auxiliar local (reutilizando lógica similar a ghost.c)
static bool IsMapTileValid(Game* game, int gridX, int gridY) {
    if (gridX < 0 || gridX >= game->mapa->colunas) return false;
    if (gridY < 0 || gridY >= game->mapa->linhas) return false;
    if (game->mapa->matriz[gridY][gridX] == PAREDE) return false;
    return true;
}

void InitPacman(Game* game) {
    if (game->mapa == NULL) return;

    // Configura posição inicial baseada no mapa
    game->pacman.gridPos = game->mapa->pacman_inicio;
    
    // Converte grid para pixels (Vector2)
    game->pacman.pixelPos.x = (float)(game->pacman.gridPos.x * TAMANHO_BLOCO);
    game->pacman.pixelPos.y = (float)(game->pacman.gridPos.y * TAMANHO_BLOCO);

    // Estado inicial
    game->pacman.currentDir = (Direcao){0, 0}; // Parado
    game->pacman.nextDir = (Direcao){0, 0};
    
    // Se for novo jogo (score == 0), reseta vidas. 
    // Se for respawn após morte, mantém score e vidas (tratado no main/game loop)
    if (game->pacman.score == 0 && game->pacman.lives == 0) {
        game->pacman.lives = 3;
        game->pacman.score = 0;
    }

    game->pacman.isPoweredUp = false;
    game->pacman.powerUpTimer = 0.0f;
}

void UpdatePacman(Game* game, float delta) {
    Pacman* p = &game->pacman;
    float speed = 5.0f * TAMANHO_BLOCO; // Pacman ligeiramente mais rápido que fantasmas normais
    float margemCentro = 2.0f;

    // --- 1. Input do Jogador ---
    if (IsKeyDown(KEY_UP))    p->nextDir = (Direcao){0, -1};
    if (IsKeyDown(KEY_DOWN))  p->nextDir = (Direcao){0, 1};
    if (IsKeyDown(KEY_LEFT))  p->nextDir = (Direcao){-1, 0};
    if (IsKeyDown(KEY_RIGHT)) p->nextDir = (Direcao){1, 0};

    // --- 2. Lógica de Movimento e Grade ---
    int gridX = (int)((p->pixelPos.x + TAMANHO_BLOCO / 2) / TAMANHO_BLOCO);
    int gridY = (int)((p->pixelPos.y + TAMANHO_BLOCO / 2) / TAMANHO_BLOCO);
    
    // Posição central exata do tile atual
    float centroX = gridX * TAMANHO_BLOCO;
    float centroY = gridY * TAMANHO_BLOCO;
    
    // Distância atual até o centro do tile
    float dist = fabs(p->pixelPos.x - centroX) + fabs(p->pixelPos.y - centroY);

    // Se estiver próximo do centro do bloco, pode tentar mudar de direção
    if (dist < margemCentro) {
        // Tenta virar para a direção desejada (nextDir)
        if (IsMapTileValid(game, gridX + p->nextDir.x, gridY + p->nextDir.y)) {
            p->currentDir = p->nextDir;
            // Centraliza forçadamente para evitar drift (desalinhamento) ao virar
            if (p->nextDir.x != 0) p->pixelPos.y = centroY;
            if (p->nextDir.y != 0) p->pixelPos.x = centroX;
        }
        
        // Verifica se pode continuar na direção atual (se não for parede)
        if (!IsMapTileValid(game, gridX + p->currentDir.x, gridY + p->currentDir.y)) {
            // Se for parede à frente, para e centraliza
            p->pixelPos.x = centroX;
            p->pixelPos.y = centroY;
            p->currentDir = (Direcao){0, 0};
        }
    }

    // Aplica movimento
    p->pixelPos.x += p->currentDir.x * speed * delta;
    p->pixelPos.y += p->currentDir.y * speed * delta;

    // Atualiza gridPos para lógica de jogo
    p->gridPos.x = gridX;
    p->gridPos.y = gridY;

    // --- 3. Portais (Teleporte) ---
    // Checa se saiu dos limites da tela (Esquerda/Direita)
    if (p->pixelPos.x < -TAMANHO_BLOCO) {
        p->pixelPos.x = (game->mapa->colunas * TAMANHO_BLOCO);
    } 
    else if (p->pixelPos.x > game->mapa->colunas * TAMANHO_BLOCO) {
        p->pixelPos.x = -TAMANHO_BLOCO;
    }

    // --- 4. Interação com o Mapa (Comer) ---
    if (gridX >= 0 && gridX < game->mapa->colunas && gridY >= 0 && gridY < game->mapa->linhas) {
        Blocos blocoAtual = game->mapa->matriz[gridY][gridX];

        if (blocoAtual == PELLET) {
            game->mapa->matriz[gridY][gridX] = VAZIO;
            p->score += 10;
            if (game->pelletsRemaining > 0) game->pelletsRemaining--;
        } 
        else if (blocoAtual == POWER_PELLET) {
            game->mapa->matriz[gridY][gridX] = VAZIO;
            p->score += 50;
            p->isPoweredUp = true;
            p->powerUpTimer = 8.0f; // Duração do power-up
            ScareGhosts(game); // Chama função do ghost.c
        }
    }

    // Atualiza timer do Power Up
    if (p->isPoweredUp) {
        p->powerUpTimer -= delta;
        if (p->powerUpTimer <= 0) {
            p->isPoweredUp = false;
        }
    }

    // --- 5. Colisão com Fantasmas ---
    // Usando colisão simples AABB ou raio (círculo)
    float raioColisao = TAMANHO_BLOCO / 2.5f;

    for (int i = 0; i < game->ghostCount; i++) {
        if (!game->ghosts[i].isActive) continue;

        // Distância entre Pacman e Fantasma
        float dx = p->pixelPos.x - game->ghosts[i].position.x;
        float dy = p->pixelPos.y - game->ghosts[i].position.y;
        float distancia = sqrtf(dx*dx + dy*dy);

        // Se tocou (distância menor que soma dos raios, aprox tamanho de 1 bloco)
        if (distancia < TAMANHO_BLOCO) {
            if (game->ghosts[i].isVulnerable) {
                // Come o fantasma
                game->ghosts[i].isActive = false; // Ou mandar para "casa" (resetar posição)
                p->score += 200;
            } else {
                // Pacman morre
                p->lives--;
                
                // Reinicia posições (Reset simples)
                InitGhosts(game); // Reseta fantasmas
                
                // Reseta posição do Pacman, mas mantém vidas/score
                p->gridPos = game->mapa->pacman_inicio;
                p->pixelPos.x = (float)(p->gridPos.x * TAMANHO_BLOCO);
                p->pixelPos.y = (float)(p->gridPos.y * TAMANHO_BLOCO);
                p->currentDir = (Direcao){0, 0};
                p->nextDir = (Direcao){0, 0};

                if (p->lives <= 0) {
                    game->gameOver = true; // Flag para o Main tratar Game Over
                }
            }
        }
    }
}

void DrawPacman(Game* game) {
    int offset = TAMANHO_BLOCO / 2;
    float raio = (TAMANHO_BLOCO / 2) - 2;
    
    Color corPacman = YELLOW;
    
    // Efeito visual simples se estiver "poderoso" (piscar ou mudar cor)
    if (game->pacman.isPoweredUp) {
        // Piscar nos últimos 2 segundos
        if (game->pacman.powerUpTimer > 2.0f || (int)(game->pacman.powerUpTimer * 10) % 2 == 0) {
            corPacman = ORANGE; 
        }
    }

    // Desenha o corpo
    DrawCircle((int)game->pacman.pixelPos.x + offset, 
               (int)game->pacman.pixelPos.y + offset, 
               raio, corPacman);

    // Opcional: Desenhar boca ou direção simples baseada em currentDir
    // Para simplificar, desenhamos apenas o círculo amarelo.
}