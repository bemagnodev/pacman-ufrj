#include "raylib.h"
#include "game.h"
#include "ghost.h"
#include "gamestate.h"
#include "persistence.h"
#include "pacman.h"
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    const int screenWidth = COLUNAS * TAMANHO_BLOCO; 
    const int screenHeight = (LINHAS * TAMANHO_BLOCO) + 40; 

    InitWindow(screenWidth, screenHeight, "Pac-Man - Jogo Completo");
    SetTargetFPS(60);

    // Inicializa estruturas do jogo
    Game game = {0};
    GameState state = {0};
    
    // Inicializa mensagem de ranking vazia
    game.rankingMessage[0] = '\0';
    
    // Inicia o estado do jogo
    InitGameState(&game, &state);
    
    // Flags para controle do menu de pausa
    bool shouldQuit = false;
    bool shouldStartNew = false;

    // CORREÇÃO: Variável para controlar debounce do TAB
    bool tabWasPressed = false;

    // ===== LOOP PRINCIPAL DO JOGO =====
    while (!WindowShouldClose() && !shouldQuit) {
        float delta = GetFrameTime();

        // ===== INPUT GLOBAL =====
        
        // Tecla TAB para pausar (apenas durante o jogo)
        if (state.currentScreen == SCREEN_PLAYING) {
            bool tabIsPressed = IsKeyDown(KEY_TAB);
            
            // Detecta a BORDA de subida (momento que pressiona)
            if (tabIsPressed && !tabWasPressed) {
                game.isPaused = !game.isPaused;
            }
            
            tabWasPressed = tabIsPressed;
        }

        // ===== ATUALIZAÇÃO =====
        
        if (game.isPaused) {
            // Se estiver pausado, apenas gerencia o menu
            HandlePauseMenu(&game, &shouldQuit, &shouldStartNew);
            
            // Se deve começar novo jogo
            if (shouldStartNew) {
                StartNewGame(&game, &state);
                shouldStartNew = false;
                game.isPaused = false; // IMPORTANTE: Despause após novo jogo
            }
        } 
        else {
            // Atualiza o estado do jogo (colisões, vitória, derrota)
            UpdateGameState(&game, &state, delta);
            
            // Atualiza elementos apenas durante a gameplay
            if (state.currentScreen == SCREEN_PLAYING && game.mapa != NULL) {
                
                // ===== MOVIMENTAÇÃO DO PAC-MAN =====
                UpdatePacman(&game, delta);
                
                // ===== ATUALIZAÇÃO DOS FANTASMAS =====
                UpdateGhosts(&game, delta);
                
                // ===== TESTE TEMPORÁRIO (pode remover depois) =====
                if (IsKeyPressed(KEY_P)) {
                    ScareGhosts(&game);
                }
            }
        }

        // ===== RENDERIZAÇÃO =====
        
        BeginDrawing();
        
        ClearBackground(BLACK);
        
        // Desenha baseado na tela atual
        switch (state.currentScreen) {
            case SCREEN_MENU:
                DrawMenuScreen();
                break;
                
            case SCREEN_PLAYING:
                // Desenha o jogo
                if (game.mapa != NULL) {
                    printarMapa(game.mapa);
                    DrawGhosts(&game);
                    DrawPacman(&game);
                    
                    // Desenha HUD
                    DrawHUD(&game, &state);
                }
                
                // Se estiver pausado, desenha menu por cima
                if (game.isPaused) {
                    DrawPauseOverlay(&game, &shouldQuit, &shouldStartNew);
                }
                break;
                
            case SCREEN_GAME_OVER:
                // Desenha o jogo de fundo
                if (game.mapa != NULL) {
                    printarMapa(game.mapa);
                    DrawGhosts(&game);
                    DrawPacman(&game);
                }
                DrawGameOverScreen(&game);
                break;
                
            case SCREEN_VICTORY:
                // Desenha o jogo de fundo
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

    // ===== LIMPEZA =====
    
    UnloadGhosts(&game);
    
    if (game.mapa != NULL) {
        descartarMapa(game.mapa);
    }

    CloseWindow();

    return 0;
}

