#include "pacman.h"
#include <math.h>

void InitPacman(Game* game) {
    if (game->mapa == NULL) return;
    
    // Inicializa posição
    game->pacman.gridPos = game->mapa->pacman_inicio;
    game->pacman.pixelPos.x = game->pacman.gridPos.x * TAMANHO_BLOCO;
    game->pacman.pixelPos.y = game->pacman.gridPos.y * TAMANHO_BLOCO;
    
    // Zera as direções
    game->pacman.direcaoAtual = (Posicao){0, 0};
    game->pacman.proximaDirecao = (Posicao){0, 0};
}

// Verifica se uma posição da grade é válida para andar
bool IsPacmanValidTile(Game* game, int gridX, int gridY) {
    if (game->mapa == NULL) return false;
    if (gridX < 0 || gridX >= game->mapa->colunas) return false;
    if (gridY < 0 || gridY >= game->mapa->linhas) return false;
    
    // Pac-Man não anda na parede
    if (game->mapa->matriz[gridY][gridX] == PAREDE) return false;
    
    return true;
}

void UpdatePacman(Game* game, float delta) {
    if (game->mapa == NULL) return;

    // === 1. INPUT ===
    bool inputDetected = false;
    Posicao intencao = game->pacman.proximaDirecao;

    if (IsKeyPressed(KEY_RIGHT)) { intencao = (Posicao){1, 0}; inputDetected = true; }
    else if (IsKeyPressed(KEY_LEFT)) { intencao = (Posicao){-1, 0}; inputDetected = true; }
    else if (IsKeyPressed(KEY_UP)) { intencao = (Posicao){0, -1}; inputDetected = true; }
    else if (IsKeyPressed(KEY_DOWN)) { intencao = (Posicao){0, 1}; inputDetected = true; }
    
    if (!inputDetected) {
        if (IsKeyDown(KEY_RIGHT)) intencao = (Posicao){1, 0};
        else if (IsKeyDown(KEY_LEFT)) intencao = (Posicao){-1, 0};
        else if (IsKeyDown(KEY_UP)) intencao = (Posicao){0, -1};
        else if (IsKeyDown(KEY_DOWN)) intencao = (Posicao){0, 1};
    }
    game->pacman.proximaDirecao = intencao;

    // === 2. INVERSÃO IMEDIATA ===
    if (game->pacman.direcaoAtual.x != 0 && game->pacman.proximaDirecao.x == -game->pacman.direcaoAtual.x) {
        game->pacman.direcaoAtual = game->pacman.proximaDirecao;
    }
    if (game->pacman.direcaoAtual.y != 0 && game->pacman.proximaDirecao.y == -game->pacman.direcaoAtual.y) {
        game->pacman.direcaoAtual = game->pacman.proximaDirecao;
    }

    // === 3. MOVIMENTO ===
    float speed = PACMAN_SPEED * TAMANHO_BLOCO; 
    game->pacman.pixelPos.x += game->pacman.direcaoAtual.x * speed * delta;
    game->pacman.pixelPos.y += game->pacman.direcaoAtual.y * speed * delta;

    // === 4. LÓGICA DE GRADE ===
    float centerX = game->pacman.pixelPos.x + (TAMANHO_BLOCO / 2.0f);
    float centerY = game->pacman.pixelPos.y + (TAMANHO_BLOCO / 2.0f);
    
    int currentGridX = (int)(centerX / TAMANHO_BLOCO);
    int currentGridY = (int)(centerY / TAMANHO_BLOCO);

    game->pacman.gridPos.x = currentGridX;
    game->pacman.gridPos.y = currentGridY;

    float tileCenterX = currentGridX * TAMANHO_BLOCO;
    float tileCenterY = currentGridY * TAMANHO_BLOCO;
    float dist = fabs(game->pacman.pixelPos.x - tileCenterX) + fabs(game->pacman.pixelPos.y - tileCenterY);

    if (dist < 4.0f) {
        bool canTurn = IsPacmanValidTile(game, currentGridX + game->pacman.proximaDirecao.x, currentGridY + game->pacman.proximaDirecao.y);
        
        if (game->pacman.proximaDirecao.x != game->pacman.direcaoAtual.x || 
            game->pacman.proximaDirecao.y != game->pacman.direcaoAtual.y) {
            
            if (game->pacman.proximaDirecao.x != 0 || game->pacman.proximaDirecao.y != 0) {
                if (canTurn) {
                    game->pacman.pixelPos.x = tileCenterX;
                    game->pacman.pixelPos.y = tileCenterY;
                    game->pacman.direcaoAtual = game->pacman.proximaDirecao;
                }
            }
        }
        bool canContinue = IsPacmanValidTile(game, currentGridX + game->pacman.direcaoAtual.x, currentGridY + game->pacman.direcaoAtual.y);
        if (!canContinue) {
            game->pacman.pixelPos.x = tileCenterX;
            game->pacman.pixelPos.y = tileCenterY;
            game->pacman.direcaoAtual = (Posicao){0, 0};
        }
    }
    
    // Tratamento de Portais
    if (game->mapa->matriz[currentGridY][currentGridX] == PORTAL) {
         if (game->mapa->numPortais >= 2) {
            int idx = -1;
            for(int i=0; i<game->mapa->numPortais; i++) {
                if (game->mapa->portais[i].x == currentGridX && game->mapa->portais[i].y == currentGridY) {
                    idx = i; break;
                }
            }
            if (idx != -1) {
                int dest = (idx + 1) % game->mapa->numPortais;
                game->pacman.gridPos.x = game->mapa->portais[dest].x;
                game->pacman.gridPos.y = game->mapa->portais[dest].y;
                game->pacman.pixelPos.x = game->pacman.gridPos.x * TAMANHO_BLOCO;
                game->pacman.pixelPos.y = game->pacman.gridPos.y * TAMANHO_BLOCO;
                game->pacman.pixelPos.x += game->pacman.direcaoAtual.x * (TAMANHO_BLOCO / 2.0f); 
                game->pacman.pixelPos.y += game->pacman.direcaoAtual.y * (TAMANHO_BLOCO / 2.0f);
            }
         }
    }
}

void DrawPacman(Game* game) {
    Vector2 center = { 
        game->pacman.pixelPos.x + (TAMANHO_BLOCO / 2), 
        game->pacman.pixelPos.y + (TAMANHO_BLOCO / 2) 
    };
    
    float raio = TAMANHO_BLOCO * 0.45f;
    
    // 1. Calcula o ângulo base baseado na direção
    float rotation = 0.0f;
    if (game->pacman.direcaoAtual.x == 1) rotation = 0.0f;
    else if (game->pacman.direcaoAtual.x == -1) rotation = 180.0f;
    else if (game->pacman.direcaoAtual.y == -1) rotation = 270.0f;
    else if (game->pacman.direcaoAtual.y == 1) rotation = 90.0f;
    
    // 2. Animação da boca (VELOCIDADE ATUALIZADA: 20.0f)
    // Se quiser ainda mais rápido, aumente para 30.0f
    float mouthOpen = fabs(sinf(GetTime() * 20.0f)) * 45.0f;
    
    // Se estiver parado, mantém a boca aberta (estética clássica)
    if (game->pacman.direcaoAtual.x == 0 && game->pacman.direcaoAtual.y == 0) {
        mouthOpen = 45.0f; 
    }

    // 3. Desenha o setor do círculo (Pac-Man)
    float startAngle = rotation + mouthOpen;
    float endAngle = rotation + 360.0f - mouthOpen;
    
    DrawCircleSector(center, raio, startAngle, endAngle, 30, YELLOW);
}