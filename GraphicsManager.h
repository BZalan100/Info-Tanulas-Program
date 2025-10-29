#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

class GraphicsManager
{
private:
    GraphicsManager() = default;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* button_font = nullptr;
    TTF_Font* code_font = nullptr;
    SDL_Texture* switch_texture = nullptr;
    SDL_Texture* button_texture = nullptr;
    SDL_Texture* wheel_texture = nullptr;
public:
    static GraphicsManager& instance()
    {
        static GraphicsManager instance;
        return instance;    
    }

    GraphicsManager(const GraphicsManager&) = delete;
    GraphicsManager& operator=(const GraphicsManager&) = delete;
    GraphicsManager(GraphicsManager&&) = delete;
    GraphicsManager& operator=(GraphicsManager&&) = delete;

    bool initialize(short width, short height)
    {
        if (!SDL_CreateWindowAndRenderer("tanulas", width, height, 0, &window, &renderer))
        {
            SDL_Log("Nem sikerult letrehozni az ablakot/renderert: %s", SDL_GetError());
            return false;
        }
        button_font = TTF_OpenFont("buttonfont.ttf", 24);
        code_font = TTF_OpenFont("codefont.ttf", 15);
        if (!button_font || !code_font)
        {
            SDL_Log("Nem sikerult megtalalni a fontot: %s", SDL_GetError());
            return false;
        }
        switch_texture = IMG_LoadTexture(renderer, "jobb.png");
        button_texture = IMG_LoadTexture(renderer, "button.png");
        wheel_texture = IMG_LoadTexture(renderer, "wheel.png");
        if (!switch_texture || !button_texture || !wheel_texture)
        {
            SDL_Log("Nem sikerult betolteni a texturat: %s", SDL_GetError());
            return false;
        }
        SDL_SetTextureBlendMode(switch_texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(button_texture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureBlendMode(wheel_texture, SDL_BLENDMODE_BLEND);
        return true;
    }

    SDL_Window* get_window() const noexcept { return window; }
    SDL_Renderer* get_renderer() const noexcept { return renderer; }
    TTF_Font* get_button_font() const noexcept { return button_font; }
    TTF_Font* get_code_font() const noexcept { return code_font; }
    SDL_Texture* get_switch_texture() const noexcept { return switch_texture; }
    SDL_Texture* get_button_texture() const noexcept { return button_texture; }
    SDL_Texture* get_wheel_texture() const noexcept { return wheel_texture; }

    ~GraphicsManager()
    {
        if (wheel_texture) SDL_DestroyTexture(wheel_texture);
        if (button_texture) SDL_DestroyTexture(button_texture);
        if (switch_texture) SDL_DestroyTexture(switch_texture);
        if (code_font) TTF_CloseFont(code_font);
        if (button_font) TTF_CloseFont(button_font);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
    }
};
