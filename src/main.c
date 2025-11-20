#include "raylib.h"
#include "game.h"
#include "ghost.h"
#include <stdlib.h>

int main(void) {
    const int screenWidth = COLUNAS * TAMANHO_BLOCO; 
    const int screenHeight = (LINHAS * TAMANHO_BLOCO) + 40; 

    InitWindow(screenWidth, screenHeight, "Pac-Man - Teste Integracao");
    SetTargetFPS(60);

    Game game = {0};

    game.mapa = lerMapa("mapa.txt");

    if (game.mapa != NULL) {
        InitGhosts(&game);
    }

    while (!WindowShouldClose()) {
        float delta = GetFrameTime();

        // TESTE TEMPORÁRIO: Aperte 'P' para simular comer um Power Pellet
        if (IsKeyPressed(KEY_P)) {
            ScareGhosts(&game);
        }

        if (IsKeyPressed(KEY_K)) {
            // Mata o primeiro fantasma vivo que encontrar
            for(int i=0; i < game.ghostCount; i++) {
                if(game.ghosts[i].isActive) {
                    game.ghosts[i].isActive = false;
                    break;
                }
            }
        }
        ////////////////////////////////

        
        if (game.mapa != NULL) {
            UpdateGhosts(&game, delta);
        }

        BeginDrawing();
        
        ClearBackground(BLACK);

        if (game.mapa != NULL) {
            printarMapa(game.mapa);
            DrawGhosts(&game);
        }

        EndDrawing();
    }

    UnloadGhosts(&game);
    
    if (game.mapa != NULL) {
        descartarMapa(game.mapa);
    }

    CloseWindow();

    return 0;
}