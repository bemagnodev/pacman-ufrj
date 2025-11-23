#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "raylib.h"

#define LINHAS 20
#define COLUNAS 40


Mapa* lerMapa(const char *arquivo){
    FILE* f = fopen(arquivo, "r");
    if (f == NULL) {
        perror("Erro ao abrir o arquivo do mapa");
        return NULL;
    }

    Mapa* mapa = (Mapa*) malloc(sizeof(Mapa));
    if(mapa == NULL){
        perror("Erro ao alocar memória para o mapa");
        fclose(f);
        return NULL;
    }

    mapa->numPellets = 0;
    mapa->numFantasmas = 0;
    mapa->fantasmas_inicio = NULL;
    mapa->numPortais = 0;
    mapa->portais = NULL;
    mapa->linhas = LINHAS;
    mapa->colunas = COLUNAS;

    mapa->matriz = (Blocos**) malloc(sizeof(Blocos*) * mapa->linhas);
    if (mapa->matriz == NULL) {
        perror("Erro ao alocar memória para as linhas do mapa");
        free(mapa);
        fclose(f);
        return NULL;
    }

    for(int i = 0; i < mapa->linhas; i++){
        mapa->matriz[i] = (Blocos*) malloc(sizeof(Blocos) * mapa->colunas);
        if (mapa->matriz[i] == NULL) {
            perror("Erro ao alocar memória para as colunas do mapa");
            
            for (int j = 0; j < i; j++) {
                free(mapa->matriz[j]);
            }
            
            free(mapa->matriz);
            free(mapa);
            fclose(f);
            return NULL;
        }
    }

    char buffer[COLUNAS + 2]; // +2 para \n e \0
    for (int i = 0; i < mapa->linhas; i++) {
        
        if (fgets(buffer, sizeof(buffer), f) == NULL) {
            perror("Erro ao ler linha do mapa");
            descartarMapa(mapa);
            fclose(f);
            return NULL;
        }

        for (int j = 0; j < mapa->colunas; j++) {
            switch (buffer[j]) {
                case '#':
                    mapa->matriz[i][j] = PAREDE;
                    break;
                
                case '.':
                    mapa->matriz[i][j] = PELLET;
                    mapa->numPellets++;
                    break;
                
                case 'o':
                    mapa->matriz[i][j] = POWER_PELLET;
                    break;
                
                case 'P':
                    mapa->matriz[i][j] = PACMAN_INICIO;
                    mapa->pacman_inicio.x = j;
                    mapa->pacman_inicio.y = i;
                    break;
                
                case 'F':
                    mapa->matriz[i][j] = FANTASMA_INICIO;
                    mapa->numFantasmas++;
                    mapa->fantasmas_inicio = (Posicao*) realloc(mapa->fantasmas_inicio, sizeof(Posicao) * mapa->numFantasmas);
                    mapa->fantasmas_inicio[mapa->numFantasmas - 1].x = j;
                    mapa->fantasmas_inicio[mapa->numFantasmas - 1].y = i;
                    break;
                
                case 'T':
                    mapa->matriz[i][j] = PORTAL;
                    mapa->numPortais++;
                    mapa->portais = (Posicao*) realloc(mapa->portais, sizeof(Posicao) * mapa->numPortais);
                    mapa->portais[mapa->numPortais - 1].x = j;
                    mapa->portais[mapa->numPortais - 1].y = i;
                    break;
                
                default:
                    mapa->matriz[i][j] = VAZIO;
                    break;
            }
        }
    }

    fclose(f);
    return mapa;

}

void printarMapa(Mapa* mapa){
    if(mapa == NULL) return;

    int tb = TAMANHO_BLOCO; // Atalho para facilitar a leitura
    int offset = tb / 2;    // Metade do bloco (para centralizar círculos)

    for(int i = 0; i < mapa->linhas; i++){
        for(int j = 0; j < mapa->colunas; j++){
            
            int posX = j * tb;
            int posY = i * tb;

            if(mapa->matriz[i][j] == PAREDE){
                DrawRectangle(posX, posY, tb, tb, BLUE);
            }

            else if(mapa->matriz[i][j] == PORTAL){
                DrawRectangle(posX, posY, tb, tb, PINK);
            }

            else if(mapa->matriz[i][j] == POWER_PELLET){
                // Fundo preto (limpar parede se houver)
                // DrawRectangle(posX, posY, tb, tb, BLACK); 
                DrawCircle(posX + offset, posY + offset, tb/3, GREEN); // Raio proporcional
            }

            else if(mapa->matriz[i][j] == PELLET){
                // DrawRectangle(posX, posY, tb, tb, BLACK);
                DrawCircle(posX + offset, posY + offset, tb/8, WHITE); // Raio bem pequeno
            }
            
            // VAZIO não precisa desenhar nada (já é o fundo preto da janela)
        }
    }
}

void descartarMapa(Mapa* mapa) {
    if (mapa == NULL) return;

    for (int i = 0; i < mapa->linhas; i++) {
        free(mapa->matriz[i]);
    }
    free(mapa->matriz);
    free(mapa->fantasmas_inicio);
    free(mapa->portais);
    free(mapa);
}