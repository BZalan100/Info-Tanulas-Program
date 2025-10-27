#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>
#include "GraphicsManager.h"
#include "Button.h"

#include <fstream>
#include <string>
#include <string_view>
#include <array>
#include <random>
#include <ctime>
#include <utility>

namespace window_size
{
    constexpr short WIDTH = 1280;
    constexpr short HEIGHT = 720;
}

namespace colors
{
    constexpr short ROWS = 10;
    constexpr short COLS = 6;
}

constexpr short DEFINITION_COUNT = 56;
constexpr short PROGRAMS_COUNT = 24;
constexpr std::string_view HIDDEN_TEXT = "#- ide lehet irni -#";

std::array<Button, DEFINITION_COUNT> def_buttons;
std::array<Button, PROGRAMS_COUNT+4> prog_buttons;
Button definitions_button, programs_button, back_button,
hint_button, show_colors_button, switch_right_button, switch_left_button;
int current_page = 1;

std::array<std::array<Button, colors::COLS>, colors::ROWS> color_buttons;
SDL_Color code_color = {0, 255, 0, 255};
bool color_panel_on = false;

std::array<std::string, DEFINITION_COUNT> titles, contents;
std::array<std::string, 40> rendered_code_lines{};
std::pair<std::string, int> hidden_line{"", 0};

int getRandomNumber(int x)
{
    static std::mt19937_64 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> distInt(0, x-1);
    return distInt(rng);
}

void getTitlesAndContents()
{
    std::ifstream f("definiciok.txt");
    for (int i = 0; i < DEFINITION_COUNT; i++)
    {
        std::string title, content;
        getline(f, title);
        getline(f, content);
        titles[i] = title;
        contents[i] = content;
    }
}

void setUpColorButtons()
{
    for (int i = 0; i < colors::ROWS; i++)
    {
        for (int j = 0; j < colors::COLS; j++)
        {
            color_buttons[i][j].setRect(1000+20*j, 500+20*i, 20, 20);
            
            SDL_Color color = {0, 0, 0, 255};
            color.r = j * (255 / (colors::COLS - 1));
            color.g = i * (255 / (colors::ROWS - 1));
            color.b = 128;

            color_buttons[i][j].setColor(color.r, color.g, color.b, 255, ColorRole::color_normal);
            color_buttons[i][j].setColor(color.r, color.g, color.b, 255, ColorRole::color_hover);
            color_buttons[i][j].setColor(color.r, color.g, color.b, 255, ColorRole::color_pressed);
            color_buttons[i][j].setOnClick([color](){
                code_color = color;
            });
        }
    }
    color_panel_on = true;
}

void readProgram(int x)
{
    std::string index = std::to_string(x);
    std::ifstream f("programok/"+index);
    std::string line;
    int i = 0;
    if (!f.is_open()) {
    SDL_Log("Failed to open file!");
    return;}
    while (std::getline(f, line))
        rendered_code_lines[i++] = line;
    int random_index = getRandomNumber(i);
    while (rendered_code_lines[random_index].size() < 4)
        random_index = getRandomNumber(i);
    hidden_line = {rendered_code_lines[random_index], random_index};
    rendered_code_lines[random_index] = HIDDEN_TEXT;
}

void setUpDefinitions(int page);
void setUpPrograms();

void setUpMainMenu()
{
    definitions_button.setRect(540, 320, 250, 50);
    definitions_button.setFont(GraphicsManager::instance().get_button_font());
    definitions_button.setText("Definiciok");
    definitions_button.setOnClick([](){
        setUpDefinitions(current_page);
        definitions_button.hide();
        programs_button.hide();
    });
    programs_button.setRect(540, 420, 250, 50);
    programs_button.setFont(GraphicsManager::instance().get_button_font());
    programs_button.setText("Programok");
    programs_button.setOnClick([](){
        setUpPrograms();
        definitions_button.hide();
        programs_button.hide();
    });
}

void setUpDefinitions(int page)
{
    switch_left_button.setRect(40, 40, 36, 36);
    switch_left_button.setOnClick([](){
        current_page = 1;
        setUpDefinitions(current_page);
    });
    switch_right_button.setRect(1200, 40, 36, 36);
    switch_right_button.setOnClick([](){
        current_page = 2;
        setUpDefinitions(current_page);
    });

    for (int i = 0; i < DEFINITION_COUNT; i++)
    {
        def_buttons[i].setRect(85*page + 300 * (i/7) - (page-1)*window_size::WIDTH,
                               100 + 75 * (i%7),
                               200,
                               35);
        def_buttons[i].setFont(GraphicsManager::instance().get_button_font());
        def_buttons[i].setText(titles[i]);
        def_buttons[i].setOnClick([i](){
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, &titles[i][0], &contents[i][0], GraphicsManager::instance().get_window());
        });
    }
    back_button.setRect(1180, 680, 100, 40);
    back_button.setFont(GraphicsManager::instance().get_button_font());
    back_button.setText("Vissza");
    back_button.setOnClick([](){
        for (int i = 0; i < DEFINITION_COUNT; i++) def_buttons[i].hide();
        switch_left_button.setRect(0, 0, 0, 0);
        switch_right_button.setRect(0, 0, 0, 0);
        setUpMainMenu();
        back_button.hide();
    });
}

void drawLineOnScreen(std::string& text, TTF_Font* font, SDL_Color text_color, int posx, int posy)
{
    if (text.empty()) return;
    if (text == HIDDEN_TEXT)
        text_color = {255, 50, 50, 255};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), text.size(), text_color);
    if (!surface)
    {
        SDL_Log("Nem sikerult letrehozni a surface-t: %s", SDL_GetError());
        return;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(GraphicsManager::instance().get_renderer(), surface);
    SDL_FRect text_rect;
    text_rect.w = surface->w;
    text_rect.h = surface->h;
    if (posy > window_size::HEIGHT-50)
    {
        posy = posy - window_size::HEIGHT + 50;
        posx = window_size::WIDTH / 2;
    }
    text_rect.x = posx;
    text_rect.y = posy;
    SDL_RenderTexture(GraphicsManager::instance().get_renderer(), tex, nullptr, &text_rect);
    SDL_DestroyTexture(tex);
    SDL_DestroySurface(surface);
}

void hidePrograms()
{
    for (int i = 0; i < PROGRAMS_COUNT+4; i++)
        prog_buttons[i].hide();
}

void getProgramRequest(int i)
{
    std::string path = "kerelmek/";
    std::string id = std::to_string(i);
    path += id;
    std::ifstream f(path);
    if (!f.is_open())
    {
        SDL_Log(id.c_str());
        SDL_Log(path.c_str());
    }
    char kerlek[200];
    f.getline(kerlek, 200);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Kérelem", &kerlek[0], GraphicsManager::instance().get_window());
}

void hideColorButtons()
{
    for (int i = 0; i < colors::ROWS; i++)
        for (int j = 0; j < colors::COLS; j++)
            color_buttons[i][j].hide();
    color_panel_on = false;
}

void setUpShowColorsButton()
{
    show_colors_button.setRect(1180, 630, 100, 40);
    show_colors_button.setFont(GraphicsManager::instance().get_button_font());
    show_colors_button.setText("Szinek");
    show_colors_button.setOnClick([](){
        if (!color_panel_on)
            setUpColorButtons();  
        else
            hideColorButtons();
    });
}

void setUpPrograms()
{
    prog_buttons[0].setText("Easy");
    prog_buttons[7].setText("Medium");
    prog_buttons[14].setText("Hacker");
    prog_buttons[21].setText("String");
    for (int i = 0; i < PROGRAMS_COUNT+4; i++)
    {
        prog_buttons[i].setRect(100 + 300 * (i/7), 100 + 75 * (i%7), 200, 40);
        prog_buttons[i].setFont(GraphicsManager::instance().get_button_font());
        std::string program_text = std::to_string(i - i/7);
        if (i%7 != 0)
        {
            prog_buttons[i].setText(program_text);
            prog_buttons[i].setOnClick([i](){
                SDL_StartTextInput(GraphicsManager::instance().get_window());
                setUpShowColorsButton();
                getProgramRequest(i - i/7);
                readProgram(i - i/7);
                hidePrograms();     
                back_button.setOnClick([](){
                    hideColorButtons();
                    show_colors_button.hide();
                    hint_button.hide();
                    rendered_code_lines = {};
                    setUpPrograms();
                });
                hint_button.setRect(1180, 580, 100, 40);
                hint_button.setFont(GraphicsManager::instance().get_button_font());
                hint_button.setText("Mutasd");
                hint_button.setOnClick([](){
                    SDL_StopTextInput(GraphicsManager::instance().get_window());
                    const char* p = hidden_line.first.c_str();
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Megoldás", p, GraphicsManager::instance().get_window());
                    rendered_code_lines[hidden_line.second] = hidden_line.first;
                    hint_button.hide();
                });
            });
        }
    }
    back_button.setRect(1180, 680, 100, 40);
    back_button.setFont(GraphicsManager::instance().get_button_font());
    back_button.setText("Vissza");
    back_button.setOnClick([](){
        SDL_StopTextInput(GraphicsManager::instance().get_window());
        setUpMainMenu();
        hidePrograms();
        back_button.hide();
    });
}

void setAllButtonTextures()
{
    definitions_button.setTexture(GraphicsManager::instance().get_button_texture());
    programs_button.setTexture(GraphicsManager::instance().get_button_texture());
    back_button.setTexture(GraphicsManager::instance().get_button_texture());
    hint_button.setTexture(GraphicsManager::instance().get_button_texture());
    show_colors_button.setTexture(GraphicsManager::instance().get_button_texture());
    switch_right_button.setTexture(GraphicsManager::instance().get_switch_texture());
    switch_left_button.setTexture(GraphicsManager::instance().get_switch_texture());
    switch_left_button.setFlipMode(SDL_FLIP_HORIZONTAL);
    for (int i = 0; i < DEFINITION_COUNT; i++)
        def_buttons[i].setTexture(GraphicsManager::instance().get_button_texture());
    for (int i = 0; i < PROGRAMS_COUNT + 4; i++)
        prog_buttons[i].setTexture(GraphicsManager::instance().get_button_texture());
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    SDL_SetAppMetadata("Tanulas", "1.0", "tanulas");
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL inicializalas hiba: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (!TTF_Init())
    {
        SDL_Log("Nem sikerult inicializalni a fontot: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (!GraphicsManager::instance().initialize(window_size::WIDTH, window_size::HEIGHT))
    {
        return SDL_APP_FAILURE;
    }
    SDL_SetWindowResizable(GraphicsManager::instance().get_window(), 0);
    //SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_STRETCH);

    SDL_SetRenderDrawBlendMode(GraphicsManager::instance().get_renderer(), SDL_BLENDMODE_BLEND);

    setAllButtonTextures();
    setUpMainMenu();
    getTitlesAndContents();  

    return SDL_APP_CONTINUE;
}

void handleAllButtons(SDL_Event* event)
{
    definitions_button.handleEvent(*event);
    programs_button.handleEvent(*event);
    back_button.handleEvent(*event);
    hint_button.handleEvent(*event);
    show_colors_button.handleEvent(*event);
    switch_right_button.handleEvent(*event);
    switch_left_button.handleEvent(*event);
    for (int i = 0; i < DEFINITION_COUNT; i++)
        def_buttons[i].handleEvent(*event);
    for (int i = 0; i < PROGRAMS_COUNT + 4; i++)
        prog_buttons[i].handleEvent(*event);
    for (int i = 0; i < colors::ROWS; i++)
        for (int j = 0; j < colors::COLS; j++)
            color_buttons[i][j].handleEvent(*event);
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;
    else if (event->type == SDL_EVENT_TEXT_INPUT)
    {
        if (rendered_code_lines[hidden_line.second] == HIDDEN_TEXT)
            rendered_code_lines[hidden_line.second] = "";
        rendered_code_lines[hidden_line.second] += event->text.text;
    }
    else if (event->type == SDL_EVENT_KEY_DOWN)
    {
        SDL_Keycode kc = event->key.key;
        SDL_Keymod mods = event->key.mod;
        if (kc == SDLK_BACKSPACE)
        {
            if (rendered_code_lines[hidden_line.second].size() > 0)
            {
                if (rendered_code_lines[hidden_line.second] == HIDDEN_TEXT)
                    rendered_code_lines[hidden_line.second] = "";
                else rendered_code_lines[hidden_line.second].pop_back();
            }
        }
        else if (kc == SDLK_RETURN || kc == SDLK_KP_ENTER)
        {
            SDL_StopTextInput(GraphicsManager::instance().get_window());
            const char* p = hidden_line.first.c_str();
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Megoldás", p, GraphicsManager::instance().get_window());
            rendered_code_lines[hidden_line.second] = hidden_line.first;
            hint_button.hide();
        }
    }
    handleAllButtons(event);
    return SDL_APP_CONTINUE;
}

void drawAllButtons()
{
    definitions_button.draw(GraphicsManager::instance().get_renderer());
    programs_button.draw(GraphicsManager::instance().get_renderer());
    back_button.draw(GraphicsManager::instance().get_renderer());
    hint_button.draw(GraphicsManager::instance().get_renderer());
    show_colors_button.draw(GraphicsManager::instance().get_renderer());
    switch_left_button.draw(GraphicsManager::instance().get_renderer());
    switch_right_button.draw(GraphicsManager::instance().get_renderer());
    for (int i = 0; i < DEFINITION_COUNT; i++)
        def_buttons[i].draw(GraphicsManager::instance().get_renderer());
    for (int i = 0; i < PROGRAMS_COUNT+4; i++)
        prog_buttons[i].draw(GraphicsManager::instance().get_renderer());
    for (int i = 0; i < colors::ROWS; i++)
        for (int j = 0; j < colors::COLS; j++)
            color_buttons[i][j].draw(GraphicsManager::instance().get_renderer());
}

void renderAllButtons()
{
    definitions_button.renderButton(GraphicsManager::instance().get_renderer());
    programs_button.renderButton(GraphicsManager::instance().get_renderer());
    back_button.renderButton(GraphicsManager::instance().get_renderer());
    hint_button.renderButton(GraphicsManager::instance().get_renderer());
    show_colors_button.renderButton(GraphicsManager::instance().get_renderer());
    switch_left_button.renderButton(GraphicsManager::instance().get_renderer());
    switch_right_button.renderButton(GraphicsManager::instance().get_renderer());
    for (int i = 0; i < DEFINITION_COUNT; i++)
        def_buttons[i].renderButton(GraphicsManager::instance().get_renderer());
    for (int i = 0; i < PROGRAMS_COUNT+4; i++)
        prog_buttons[i].renderButton(GraphicsManager::instance().get_renderer());
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    SDL_SetRenderDrawColor(GraphicsManager::instance().get_renderer(), 33, 33, 33, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(GraphicsManager::instance().get_renderer());
    renderAllButtons();
    drawAllButtons();
    for (int i = 0; i < 40; i++)
        drawLineOnScreen(rendered_code_lines[i], GraphicsManager::instance().get_code_font(), code_color, 0, 40*i);
    SDL_RenderPresent(GraphicsManager::instance().get_renderer());
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result){
    SDL_StopTextInput(GraphicsManager::instance().get_window());
}