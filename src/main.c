#include "raylib.h"
#include "game.h"

int main(void) {
    const int screenWidth = 1600;  
    const int screenHeight = 840;  

    InitWindow(screenWidth, screenHeight, "Pac-Man UFRJ");
    SetTargetFPS(60);

    GameState gameState = {0}; 
    InitGame(&gameState);

    while (!WindowShouldClose()) {
        UpdateGame(&gameState);
        DrawGame(&gameState);
    }

    UnloadGame(&gameState);
    CloseWindow();

    return 0;
}