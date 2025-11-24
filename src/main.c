#include "raylib.h"
#include "game.h"
#include "ghost.h"
#include "gamestate.h"
#include "persistence.h"
#include "pacman.h"
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    // Calcula o tamanho da tela baseado nas constantes do game.h
    const int screenWidth = COLUNAS * TAMANHO_BLOCO; 
    const int screenHeight = (LINHAS * TAMANHO_BLOCO) + 40; 

    InitWindow(screenWidth, screenHeight, "Pac-Man UFRJ - 2025.2");
    SetTargetFPS(60);

    // Inicializa estruturas do jogo
    Game game = {0};
    GameState state = {0};
    
    // Inicializa mensagem de ranking vazia para evitar lixo de memória
    game.rankingMessage[0] = '\0';
    
    // Inicia o estado do jogo (Carrega mapa, zera scores, etc.)
    InitGameState(&game, &state);
    
    // Flags para controle do menu de pausa
    bool shouldQuit = false;
    bool shouldStartNew = false;
    bool tabWasPressed = false;

    // ===== LOOP PRINCIPAL DO JOGO =====
    while (!WindowShouldClose() && !shouldQuit) {
        float delta = GetFrameTime();

        // ==========================================
        // 1. INPUTS GLOBAIS
        // ==========================================
        
        // Tecla TAB para pausar (apenas durante o jogo)
        if (state.currentScreen == SCREEN_PLAYING) {
            bool tabIsPressed = IsKeyDown(KEY_TAB);
            if (tabIsPressed && !tabWasPressed) {
                game.isPaused = !game.isPaused;
            }
            tabWasPressed = tabIsPressed;
        }

        // ==========================================
        // 2. ATUALIZAÇÃO (UPDATE)
        // ==========================================
        if (game.saveMessageTimer > 0) {
            game.saveMessageTimer -= delta;
        }
        if (game.isPaused) {
            // Se estiver pausado, apenas gerencia o menu de pausa
            HandlePauseMenu(&game, &shouldQuit, &shouldStartNew);
            
            if (shouldStartNew) {
                StartNewGame(&game, &state);
                shouldStartNew = false;
                game.isPaused = false; 
            }
        } 
        else {
            // Lógica enquanto o jogo roda (não pausado)
            if (state.currentScreen == SCREEN_PLAYING && game.mapa != NULL) {
                
                // Movemos as entidades
                UpdatePacman(&game, delta);
                UpdateGhosts(&game, delta);
                

                // CHEAT: Pular de Nível (Tecla L)
                if (IsKeyPressed(KEY_L)) {
                    // Força a vitória esvaziando os pellets
                    if (game.mapa != NULL) game.mapa->numPellets = 0;
                }
                
            }

            // 3º: Verificamos as regras (Colisões, Vitória, Derrota)
            // É importante chamar isso DEPOIS do movimento para a colisão ser exata
            UpdateGameState(&game, &state, delta);
        }

        // ==========================================
        // 3. RENDERIZAÇÃO (DRAW)
        // ==========================================
        
        BeginDrawing();
        ClearBackground(BLACK); // Limpa a tela
        
        switch (state.currentScreen) {
            case SCREEN_MENU:
                DrawMenuScreen();
                break;
                
            case SCREEN_PLAYING:
                if (game.mapa != NULL) {
                    printarMapa(game.mapa);
                    if (!state.isInvulnerable || (int)(GetTime() * 10) % 2 == 0) {
                        DrawPacman(&game);// Pacman desenhado antes dos fantasmas ou depois? Depois = em cima.
                    }
                    DrawGhosts(&game);
                    DrawHUD(&game, &state);
                }
                
                // Overlay de Pausa
                if (game.isPaused) {
                    DrawPauseOverlay(&game, &shouldQuit, &shouldStartNew);
                }
                break;
                
            case SCREEN_GAME_OVER:
                // Desenha o jogo congelado no fundo
                if (game.mapa != NULL) {
                    printarMapa(game.mapa);
                    DrawGhosts(&game);
                    DrawPacman(&game);
                }
                DrawGameOverScreen(&game);
                break;
                
            case SCREEN_VICTORY:
                if (game.mapa != NULL) {
                    printarMapa(game.mapa);
                    DrawGhosts(&game);
                    DrawPacman(&game);
                }
                DrawVictoryScreen(&game);
                break;
                
            default:
                break;
        }
        
        EndDrawing();
    }

    // ==========================================
    // 4. LIMPEZA DE MEMÓRIA
    // ==========================================
    
    UnloadGhosts(&game);
    if (game.mapa != NULL) {
        descartarMapa(game.mapa);
    }

    CloseWindow();

    return 0;
}