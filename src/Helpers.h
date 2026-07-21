#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>    // std::cerr — like Console.Error in C#
#include <vector>
#include <memory>      // std::unique_ptr — smart pointer for RAII
#include <algorithm> 

#include "Constants.h"

class Helpers
{
public:
    static bool DrawButton(
        SDL_Renderer* renderer,
        TTF_Font* font,
        const SDL_Rect& baseRect,
        const char* label,
        const Vec2& windowScale,
        int mouseX,
        int mouseY,
        Uint32 mouseState
    );
};