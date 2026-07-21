#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>    // std::cerr — like Console.Error in C#
#include <vector>
#include <memory>      // std::unique_ptr — smart pointer for RAII
#include <algorithm> 

#include "Constants.h"
#include "GameState.h"
#include "Helpers.h"

// ---- RAII helper -------------------------------------------------------------
// SDL uses C-style opaque pointers (SDL_Window*, SDL_Renderer*).
// If we stored them as raw pointers we'd need to call SDL_DestroyWindow etc.
// manually — easy to forget. Instead we wrap them in unique_ptr with a custom
// *deleter* so destruction is automatic when the pointer goes out of scope.
//
// unique_ptr<T, Deleter> owns the pointer exclusively — no reference counting.
// This is different from shared_ptr (which does ref-count) and raw pointers.
// Use unique_ptr by default; only escalate to shared_ptr when ownership is shared.
struct SdlWindowDeleter   { void operator()(SDL_Window*   w) { SDL_DestroyWindow(w);   } };
struct SdlRendererDeleter { void operator()(SDL_Renderer* r) { SDL_DestroyRenderer(r); } };

using WindowPtr   = std::unique_ptr<SDL_Window,   SdlWindowDeleter>;
using RendererPtr = std::unique_ptr<SDL_Renderer, SdlRendererDeleter>;



// ---- main --------------------------------------------------------------------
// In C++ main() returns int to the OS: 0 = success, non-zero = error.
// argc/argv are the command-line arguments (we ignore them here).
int main(int /*argc*/, char* /*argv*/[]) {

    // SDL_Init returns 0 on success, negative on failure.
    // The | combines flags with a bitwise OR — common C/C++ pattern.
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init error: " << SDL_GetError() << "\n";
        return 1;
    }

    TTF_Init();
    TTF_Font* font = TTF_OpenFont("src/assets/fonts/Merisk.ttf", 128);
    if (!font)
    {
        printf("Font error: %s\n", TTF_GetError());
    }

    // unique_ptr takes ownership; SDL objects are destroyed automatically
    // when window/renderer go out of scope (end of main).
    // SDL_WINDOW_SHOWN is a flag — SDL uses bit flags extensively.
    WindowPtr window{
        SDL_CreateWindow(
            "Storm The Castle TD",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            Constants::SCREEN_W, Constants::SCREEN_H,
            SDL_WINDOW_SHOWN or SDL_WINDOW_FULLSCREEN
        )
    };

    Vec2 windowScale = {1.0f, 1.0f};
    int windowWidth, windowHeight;

    SDL_GetWindowSize(window.get(), &windowWidth, &windowHeight);
    windowScale.x = windowWidth / (float)Constants::SCREEN_W;
    windowScale.y = windowHeight / (float)Constants::SCREEN_H;

    if (!window) {
        std::cerr << "SDL_CreateWindow error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // -1 = use the first available rendering driver; SDL_RENDERER_ACCELERATED = use GPU
    RendererPtr renderer{
        SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)
    };
    // .get() returns the raw pointer inside the unique_ptr — SDL functions expect raw pointers

    if (!renderer) {
        std::cerr << "SDL_CreateRenderer error: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // ---- Game objects --------------------------------------------------------

    GameState gameState = GameState();
    gameState.state = GameStateType::MainMenu;

    // ---- Game loop -----------------------------------------------------------
    // The classic: process input → update state → render → repeat.
    bool running = true;
    Uint64 prevTime = SDL_GetTicks64();  // Uint64 = unsigned 64-bit int (SDL typedef)

    while (running) {

        // -- Delta time --
        // We compute how many seconds elapsed since last frame.
        // Capping dt prevents huge jumps if the window is moved/resized/debugged.
        Uint64 now = SDL_GetTicks64();
        float dt = (now - prevTime) / 1000.0f;  // milliseconds → seconds
        dt = (dt < 0.05f) ? dt : 0.05f;         // ternary operator — same as C#
        prevTime = now;

        // -- Event pump --
        // SDL queues up OS events (keyboard, mouse, window close, etc.).
        // SDL_PollEvent pops one event per call; loop until the queue is empty.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;  // Window close button

            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                running = false;
        }

        // -- Input --
        // SDL_GetKeyboardState returns a pointer to an internal array.
        // nullptr here means "don't tell me how many keys there are".
        int mouseX, mouseY;
        const Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        // Clear with a dark background color
        SDL_SetRenderDrawColor(renderer.get(), 200, 200, 150, 255);
        SDL_RenderClear(renderer.get());

        // Handle main menu
        if (gameState.state == GameStateType::MainMenu) {
            if (Helpers::DrawButton(
                renderer.get(),
                font,
                Constants::StartButton,
                "Start Game",
                windowScale,
                mouseX,
                mouseY,
                mouseState
            )) {
                gameState.state = GameStateType::LevelSelect;
            }
        }

        // Handle level select
        if (gameState.state == GameStateType::LevelSelect) {

            struct LevelButton {
                SDL_Rect baseRect;
                const char* label;
                GameStateType targetState;
            };

            LevelButton levelButtons[4] = {
                { Constants::CastleButton,      "Castle",       GameStateType::LevelOne },
                { Constants::GardensButton,     "Gardens",      GameStateType::LevelTwo },
                { Constants::DesertOasisButton, "Desert Oasis", GameStateType::LevelThree },
                { Constants::MountainTopButton, "Mountain Top", GameStateType::LevelFour },
            };

            for (const LevelButton& lvl : levelButtons) {
            if (Helpers::DrawButton(
                    renderer.get(),
                    font,
                    lvl.baseRect,
                    lvl.label,
                    windowScale,
                    mouseX,
                    mouseY,
                    mouseState
                )) {
                    gameState.state = lvl.targetState;
                    // e.g. gameState.selectedLevel = lvl.level;
                }
            }
        }


        // Swap front/back buffers — makes the frame visible.
        // With PRESENTVSYNC this blocks until the next monitor refresh (60Hz → ~16ms wait).
        SDL_RenderPresent(renderer.get());
    }

    // unique_ptr destructors run here, calling SDL_DestroyWindow/Renderer automatically
    SDL_Quit();
    TTF_CloseFont(font);
    TTF_Quit();
    return 0;
}
