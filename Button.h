#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include <functional>
#include <string>

enum ColorRole
{
    color_normal,
    color_hover,
    color_pressed,
};

class Button
{
    SDL_FRect rect;
    SDL_Color color_normal;
    SDL_Color color_hover;
    SDL_Color color_pressed;
    TTF_Font* font = NULL;
    std::string text = "";
    bool hovered = false;
    bool pressed = false;
    std::function<void()> onClick;
    SDL_Texture* texture = NULL;
    SDL_FlipMode flip_mode;
public:
    Button()
    {
        rect = {0, 0, 0, 0};
        color_normal = {66, 66, 66, 0};
        color_hover = {180, 180, 250, 0};
        color_pressed = {120, 120, 120, 0};
        font = NULL;
        text = "";
        hovered = false;
        pressed = false;
        onClick = [](){};
        texture = NULL;
        flip_mode = SDL_FLIP_NONE;
    }
    void draw(SDL_Renderer* renderer)
    {
        SDL_Color c = color_normal;
        if (pressed) c = color_pressed;
        else if (hovered) c = color_hover;
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(renderer, &rect);

        if (!text.empty() && font)
        {
            SDL_Color text_color = {211, 172, 31, 255};
            SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), text.size(), text_color);
            if (!surface)
            {
                SDL_Log("Nem sikerult letrehozni a surface-t: %s", SDL_GetError());
                return;
            }
            SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FRect text_rect;
            text_rect.w = surface->w;
            text_rect.h = surface->h;
            text_rect.x = static_cast<int>(rect.x + (rect.w - surface->w)/2);
            text_rect.y = static_cast<int>(rect.y + (rect.h - surface->h)/2);
            SDL_RenderTexture(renderer, tex, NULL, &text_rect);
            SDL_DestroyTexture(tex);
            SDL_DestroySurface(surface);
        }
    }
    void handleEvent(const SDL_Event& e)
    {
        if (e.type == SDL_EVENT_MOUSE_MOTION)
        {
            float mx = static_cast<float>(e.motion.x);
            float my = static_cast<float>(e.motion.y);
            hovered = (mx >= rect.x && mx < rect.x + rect.w && 
                       my >= rect.y && my < rect.y + rect.h);
        }
        else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT)
        {
            if (hovered) pressed = true;
        }
        else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && e.button.button == SDL_BUTTON_LEFT)
        {
            if (pressed && hovered && onClick) onClick();
            pressed = false;
        }
    }
    void setRect(float x, float y, float w, float h)
    {
        rect = {x, y, w, h};
    }
    void setColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a, ColorRole color)
    {
        if (color == ColorRole::color_normal) color_normal = {r, g, b, a};
        if (color == ColorRole::color_hover) color_hover = {r, g, b, a};
        if (color == ColorRole::color_pressed) color_pressed = {r, g, b, a};
    }
    void setFont(TTF_Font* f)
    {
        font = f;
    }
    void setText(const std::string& t)
    {
        text = t;
    }
    void setOnClick(const std::function<void()>& func)
    {
        onClick = func;
    }
    void setTexture(SDL_Texture* t)
    {
        texture = t;
    }
    void setFlipMode(SDL_FlipMode f)
    {
        flip_mode = f;
    }
    void renderButton(SDL_Renderer* r, const SDL_FRect* src = nullptr, const SDL_FRect* dest = nullptr, double angles = 0.0, const SDL_FPoint* center = nullptr)
    {
        if (texture == NULL) return;
        color_normal.a = color_hover.a = color_pressed.a = 0;
        SDL_SetTextureColorModFloat(texture, 1.0f, 1.0f, 1.0f);
        if (pressed)
            SDL_SetTextureColorModFloat(texture,
                                        (float)color_pressed.r / 255.0f,
                                        (float)color_pressed.g / 255.0f,
                                        (float)color_pressed.b / 255.0f); 
        else if (hovered)
            SDL_SetTextureColorModFloat(texture,
                                        (float)color_hover.r / 255.0f,
                                        (float)color_hover.g / 255.0f,
                                        (float)color_hover.b / 255.0f);     
        if (!dest) SDL_RenderTextureRotated(r, texture, src, &rect, angles, center, flip_mode);
        else SDL_RenderTextureRotated(r, texture, src, dest, angles, center, flip_mode);
    }
    void hide()
    {
        rect = {-100, -100, 0, 0};
    }
};