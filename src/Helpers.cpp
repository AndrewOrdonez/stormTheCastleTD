#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>    // std::cerr — like Console.Error in C#
#include <vector>
#include <memory>      // std::unique_ptr — smart pointer for RAII
#include <algorithm> 

#include "Constants.h"
#include "Helpers.h"

    // Returns true if the button was clicked this frame.
bool Helpers::DrawButton(
        SDL_Renderer* renderer,
        TTF_Font* font,
        const SDL_Rect& baseRect,
        const char* label,
        const Vec2& windowScale,
        int mouseX,
        int mouseY,
        Uint32 mouseState
    ) {
        SDL_Rect button = baseRect;
        button.x = static_cast<int>(button.x * windowScale.x);
        button.y = static_cast<int>(button.y * windowScale.y);
        button.w = static_cast<int>(button.w * windowScale.x);
        button.h = static_cast<int>(button.h * windowScale.y);

        SDL_Rect buttonOuter = baseRect;
        buttonOuter.x -= 5;
        buttonOuter.y -= 5;
        buttonOuter.w += 10;
        buttonOuter.h += 10;
        buttonOuter.x = static_cast<int>(buttonOuter.x * windowScale.x);
        buttonOuter.y = static_cast<int>(buttonOuter.y * windowScale.y);
        buttonOuter.w = static_cast<int>(buttonOuter.w * windowScale.x);
        buttonOuter.h = static_cast<int>(buttonOuter.h * windowScale.y);

        bool clicked = false;
        if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if (mouseX >= buttonOuter.x && mouseX <= buttonOuter.x + buttonOuter.w &&
                mouseY >= buttonOuter.y && mouseY <= buttonOuter.y + buttonOuter.h) {
                clicked = true;
            }
        }

        SDL_Surface* surface = TTF_RenderText_Blended(font, label, Constants::black);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &buttonOuter);
        SDL_SetRenderDrawColor(renderer, 100, 150, 100, 255);
        SDL_RenderFillRect(renderer, &button);
        SDL_RenderCopy(renderer, texture, nullptr, &button);

        SDL_DestroyTexture(texture);

        return clicked;
};
