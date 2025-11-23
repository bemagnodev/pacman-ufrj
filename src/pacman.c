#include "pacman.h"
#include <math.h>

// Apenas direção atual (sem "próxima direção")
static Direcao currentDirection = {0, 0};

void InitPacman(Game* game) {
    if (game->mapa == NULL) return;
    
    // Inicializa posição
    game->pacman.gridPos = game->mapa->pacman_inicio;
    game->pacman.pixelPos.x = game->pacman.gridPos.x * TAMANHO_BLOCO;
    game->pacman.pixelPos.y = game->pacman.gridPos.y * TAMANHO_BLOCO;
    
    // Reseta direção (parado)
    currentDirection.x = 0;
    currentDirection.y = 0;
}

bool IsPacmanValidTile(Game* game, int gridX, int gridY) {
    if (game->mapa == NULL) return false;
    if (gridX < 0 || gridX >= game->mapa->colunas) return false;
    if (gridY < 0 || gridY >= game->mapa->linhas) return false;
    if (game->mapa->matriz[gridY][gridX] == PAREDE) return false;
    return true;
}

void UpdatePacman(Game* game, float delta) {
    if (game->mapa == NULL) return;
    
    // NOVO: Sistema de movimento APENAS AO PRESSIONAR tecla
    // Define direção atual baseado em qual tecla está PRESSIONADA
    if (IsKeyDown(KEY_RIGHT)) {
        currentDirection.x = 1;
        currentDirection.y = 0;
    }
    else if (IsKeyDown(KEY_LEFT)) {
        currentDirection.x = -1;
        currentDirection.y = 0;
    }
    else if (IsKeyDown(KEY_UP)) {
        currentDirection.x = 0;
        currentDirection.y = -1;
    }
    else if (IsKeyDown(KEY_DOWN)) {
        currentDirection.x = 0;
        currentDirection.y = 1;
    }
    else {
        // NENHUMA tecla pressionada = Pac-Man PARA
        currentDirection.x = 0;
        currentDirection.y = 0;
        return; // Sai da função, não move
    }
    
    // Calcula posição atual no grid
    float centerX = game->pacman.pixelPos.x + (TAMANHO_BLOCO / 2.0f);
    float centerY = game->pacman.pixelPos.y + (TAMANHO_BLOCO / 2.0f);
    
    int gridX = (int)(centerX / TAMANHO_BLOCO);
    int gridY = (int)(centerY / TAMANHO_BLOCO);
    
    // Garante limites válidos
    gridX = (gridX < 0) ? 0 : (gridX >= game->mapa->colunas) ? game->mapa->colunas - 1 : gridX;
    gridY = (gridY < 0) ? 0 : (gridY >= game->mapa->linhas) ? game->mapa->linhas - 1 : gridY;
    
    // Movimento: calcula próxima posição
    float speed = PACMAN_SPEED * TAMANHO_BLOCO;
    float nextX = game->pacman.pixelPos.x + (currentDirection.x * speed * delta);
    float nextY = game->pacman.pixelPos.y + (currentDirection.y * speed * delta);
    
    // CORREÇÃO ANTI-PAREDE: Verifica colisão com margem de segurança
    float checkCenterX = nextX + (TAMANHO_BLOCO / 2.0f);
    float checkCenterY = nextY + (TAMANHO_BLOCO / 2.0f);
    
    int nextGridX = (int)(checkCenterX / TAMANHO_BLOCO);
    int nextGridY = (int)(checkCenterY / TAMANHO_BLOCO);
    
    // Verifica se pode mover
    bool canMove = true;
    
    if (nextGridX < 0 || nextGridX >= game->mapa->colunas ||
        nextGridY < 0 || nextGridY >= game->mapa->linhas) {
        canMove = false;
    }
    else if (game->mapa->matriz[nextGridY][nextGridX] == PAREDE) {
        canMove = false;
    }
    
    // Move apenas se puder
    if (canMove) {
        game->pacman.pixelPos.x = nextX;
        game->pacman.pixelPos.y = nextY;
    }
    
    
    // Atualiza posição final no grid
    centerX = game->pacman.pixelPos.x + (TAMANHO_BLOCO / 2.0f);
    centerY = game->pacman.pixelPos.y + (TAMANHO_BLOCO / 2.0f);
    gridX = (int)(centerX / TAMANHO_BLOCO);
    gridY = (int)(centerY / TAMANHO_BLOCO);
    
    gridX = (gridX < 0) ? 0 : (gridX >= game->mapa->colunas) ? game->mapa->colunas - 1 : gridX;
    gridY = (gridY < 0) ? 0 : (gridY >= game->mapa->linhas) ? game->mapa->linhas - 1 : gridY;
    
    // Verifica portais (se houver)
    if (game->mapa->numPortais >= 2 && game->mapa->matriz[gridY][gridX] == PORTAL) {
        int portalAtual = -1;
        for (int i = 0; i < game->mapa->numPortais; i++) {
            if (game->mapa->portais[i].x == gridX && game->mapa->portais[i].y == gridY) {
                portalAtual = i;
                break;
            }
        }
        
        if (portalAtual != -1) {
            int outroPortal = (portalAtual + 1) % game->mapa->numPortais;
            game->pacman.pixelPos.x = game->mapa->portais[outroPortal].x * TAMANHO_BLOCO;
            game->pacman.pixelPos.y = game->mapa->portais[outroPortal].y * TAMANHO_BLOCO;
            gridX = game->mapa->portais[outroPortal].x;
            gridY = game->mapa->portais[outroPortal].y;
        }
    }
    
    game->pacman.gridPos.x = gridX;
    game->pacman.gridPos.y = gridY;
}

void DrawPacman(Game* game) {
    int offset = TAMANHO_BLOCO / 2;
    // CORREÇÃO: Raio MENOR para não entrar nas paredes
    // Raio = 35% do bloco (7 pixels para bloco de 20)
    int raio = (int)(TAMANHO_BLOCO * 0.35f);
    
    // Desenha o Pac-Man como um círculo amarelo
    DrawCircle(
        (int)game->pacman.pixelPos.x + offset,
        (int)game->pacman.pixelPos.y + offset,
        raio,
        YELLOW
    );
    
    // Olho do Pac-Man
    Vector2 centro = {
        game->pacman.pixelPos.x + offset,
        game->pacman.pixelPos.y + offset
    };
    
    float olhoX = centro.x;
    float olhoY = centro.y - raio/3;
    
    // Ajusta posição do olho baseado na direção
    if (currentDirection.x < 0) olhoX = centro.x - raio/4;
    else if (currentDirection.x > 0) olhoX = centro.x + raio/4;
    
    DrawCircle((int)olhoX, (int)olhoY, 2, BLACK);
}