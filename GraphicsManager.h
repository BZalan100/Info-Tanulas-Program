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
        return true;
    }

    SDL_Window* get_window() const noexcept { return window; }
    SDL_Renderer* get_renderer() const noexcept { return renderer; }
    TTF_Font* get_button_font() const noexcept { return button_font; }
    TTF_Font* get_code_font() const noexcept { return code_font; }

    ~GraphicsManager()
    {
        if (code_font) TTF_CloseFont(code_font);
        if (button_font) TTF_CloseFont(button_font);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
    }
};
