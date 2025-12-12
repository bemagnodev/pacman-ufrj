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
    
    // 1. INICIALIZA O SISTEMA DE ÁUDIO
    InitAudioDevice();

    SetTargetFPS(60);

    // Inicializa estruturas do jogo
    Game game = {0};
    GameState state = {0};
    
    game.rankingMessage[0] = '\0';
    game.isMuted = false; // Começa com som

    // 2. CARREGA OS SONS
    game.sfxIntro     = LoadSound("pacman_beginning.wav");
    game.sfxWaka      = LoadSound("pacman_chomp.wav");
    game.sfxEatGhost  = LoadSound("pacman_eatghost.wav");
    game.sfxDeath     = LoadSound("pacman_death.wav");
    
    InitGameState(&game, &state);

    PlaySound(game.sfxIntro);
    
    bool shouldQuit = false;
    bool shouldStartNew = false;
    bool tabWasPressed = false;

    // ===== LOOP PRINCIPAL DO JOGO =====
    while (!WindowShouldClose() && !shouldQuit) {
        
        // --- INPUTS GLOBAIS ---

        // Tecla F11: Tela Cheia
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }

        // --- TECLA M (MUTE) ---
        if (IsKeyPressed(KEY_M)) {
            game.isMuted = !game.isMuted;
            // Se estiver mutado volume = 0, senão volume = 1
            SetMasterVolume(game.isMuted ? 0.0f : 1.0f);
        }

        float delta = GetFrameTime();
        
        // Tecla TAB para pausar
        if (state.currentScreen == SCREEN_PLAYING) {
            bool tabIsPressed = IsKeyDown(KEY_TAB);
            if (tabIsPressed && !tabWasPressed) {
                game.isPaused = !game.isPaused;
            }
            tabWasPressed = tabIsPressed;
        }

        // ATUALIZAÇÃO (UPDATE)
        if (game.saveMessageTimer > 0) {
            game.saveMessageTimer -= delta;
        }
        if (game.isPaused) {
            HandlePauseMenu(&game, &shouldQuit, &shouldStartNew);
            
            if (shouldStartNew) {
                StartNewGame(&game, &state);
                shouldStartNew = false;
                game.isPaused = false; 
            }
        } 
        else {
            if (state.currentScreen == SCREEN_PLAYING && game.mapa != NULL) {
                UpdatePacman(&game, delta);
                UpdateGhosts(&game, delta);
                
                if (IsKeyPressed(KEY_L)) {
                    if (game.mapa != NULL) game.mapa->numPellets = 0;
                }
            }
            UpdateGameState(&game, &state, delta);
        }

        // RENDERIZAÇÃO (DRAW)
        BeginDrawing();
        ClearBackground(BLACK); 
        
        switch (state.currentScreen) {
            case SCREEN_MENU:
                DrawMenuScreen();
                break;
                
            case SCREEN_PLAYING:
                if (game.mapa != NULL) {
                    printarMapa(game.mapa);
                    if (!state.isInvulnerable || (int)(GetTime() * 10) % 2 == 0) {
                        DrawPacman(&game);
                    }
                    DrawGhosts(&game);
                    DrawHUD(&game, &state);
                }
                if (game.isPaused) {
                    DrawPauseOverlay(&game, &shouldQuit, &shouldStartNew);
                }
                break;
                
            case SCREEN_GAME_OVER:
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

        // --- ÍCONE DE MUTE ---
        // Desenha um aviso pequeno no canto se estiver mudo
        if (game.isMuted) {
            DrawText("MUDO", screenWidth - 60, 10, 20, RED);
        }
        
        EndDrawing();
    }
    
    // LIMPEZA
    UnloadSound(game.sfxIntro);
    UnloadSound(game.sfxWaka);
    UnloadSound(game.sfxEatGhost);
    UnloadSound(game.sfxDeath);
    CloseAudioDevice(); 

    UnloadGhosts(&game);
    if (game.mapa != NULL) {
        descartarMapa(game.mapa);
    }

    CloseWindow();

    return 0;
}