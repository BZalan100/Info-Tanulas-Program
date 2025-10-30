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

namespace config
{
    constexpr short BUTTON_COUNT = 13;
    constexpr short DEFINITION_COUNT = 56;
    constexpr short PROGRAMS_COUNT = 24;
    constexpr std::string_view HIDDEN_TEXT = "#- ide lehet irni -#";
}

enum ButtonType
{
    definitions,
    programs,
    gamble,
    back,
    hint,
    show_colors,
    switch_left,
    switch_right,
    logo,
    black,
    red,
    green,
    wheel,
};

std::array<Button, config::BUTTON_COUNT> buttons;
std::array<Button, config::DEFINITION_COUNT> def_buttons;
std::array<Button, config::PROGRAMS_COUNT+4> prog_buttons;
std::array<std::array<Button, colors::COLS>, colors::ROWS> color_buttons;

int current_page = 1;
SDL_Color code_color = {0, 255, 0, 255};
bool color_panel_on = false;

std::array<std::string, config::DEFINITION_COUNT> titles, contents;
std::array<std::string, 40> rendered_code_lines{};
std::pair<std::string, int> hidden_line{"", 0};

bool is_spinned = false;
Uint64 spin_timer = 0;
short roullette_winner = 0;
short chosen_color = 0;
short money = 3;
std::string money_line = "";
Uint64 money_timer = 0;

int getRandomNumber(int x)
{
    static std::mt19937_64 rng(static_cast<unsigned>(std::time(nullptr)));
    std::uniform_int_distribution<int> distInt(0, x-1);
    return distInt(rng);
}

void getTitlesAndContents()
{
    std::ifstream f("definiciok.txt");
    for (int i = 0; i < config::DEFINITION_COUNT; i++)
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
    rendered_code_lines[random_index] = config::HIDDEN_TEXT;
}

void setUpDefinitions(int page);
void setUpPrograms();
void setUpGambling();

void setUpMainMenu()
{
    money_timer = SDL_GetTicks();
    money_line = "Pénz: " + std::to_string(money);

    buttons[ButtonType::logo].setRect(window_size::WIDTH / 2 - 146, 50, 293, 258);
    buttons[ButtonType::logo].setColor(255, 255, 255, 0, ColorRole::color_hover);
    buttons[ButtonType::logo].setColor(255, 255, 255, 0, ColorRole::color_pressed);

    buttons[ButtonType::definitions].setRect(window_size::WIDTH / 2 - 125, 400, 250, 50);
    buttons[ButtonType::definitions].setFont(GraphicsManager::instance().get_button_font());
    buttons[ButtonType::definitions].setText("Definiciok");
    buttons[ButtonType::definitions].setOnClick([](){
        setUpDefinitions(current_page);
        money_line = "";
        buttons[ButtonType::logo].hide();
        buttons[ButtonType::definitions].hide();
        buttons[ButtonType::programs].hide();
        buttons[ButtonType::gamble].hide();
    });
    buttons[ButtonType::programs].setRect(window_size::WIDTH / 2 - 125, 475, 250, 50);
    buttons[ButtonType::programs].setFont(GraphicsManager::instance().get_button_font());
    buttons[ButtonType::programs].setText("Programok");
    buttons[ButtonType::programs].setOnClick([](){
        setUpPrograms();
        money_line = "";
        buttons[ButtonType::logo].hide();
        buttons[ButtonType::definitions].hide();
        buttons[ButtonType::programs].hide();
        buttons[ButtonType::gamble].hide();
    });
    buttons[ButtonType::gamble].setRect(window_size::WIDTH / 2 - 125, 550, 250, 50);
    buttons[ButtonType::gamble].setFont(GraphicsManager::instance().get_button_font());
    buttons[ButtonType::gamble].setText("$$$");
    buttons[ButtonType::gamble].setOnClick([](){
        setUpGambling();
        buttons[ButtonType::logo].hide();
        buttons[ButtonType::definitions].hide();
        buttons[ButtonType::programs].hide();
        buttons[ButtonType::gamble].hide();
    });
}

void setUpGambling()
{
    buttons[ButtonType::wheel].setRect(100, 10, 588, 682);
    buttons[ButtonType::wheel].setColor(255, 255, 255, 0, ColorRole::color_hover);
    buttons[ButtonType::wheel].setColor(255, 255, 255, 0, ColorRole::color_pressed);
    buttons[ButtonType::black].setRect(1000, 30, 250, 50);
    buttons[ButtonType::black].setFont(GraphicsManager::instance().get_button_font());
    buttons[ButtonType::black].setText("Fekete");
    buttons[ButtonType::black].setOnClick([](){
        if (!is_spinned && money > 0)
        {
            spin_timer = SDL_GetTicks();
            is_spinned = true;
            chosen_color = 0;
            roullette_winner = getRandomNumber(11) / 5;
        }
    });
    buttons[ButtonType::wheel].setRect(100, 10, 588, 682);
    buttons[ButtonType::wheel].setColor(255, 255, 255, 0, ColorRole::color_hover);
    buttons[ButtonType::wheel].setColor(255, 255, 255, 0, ColorRole::color_pressed);
    buttons[ButtonType::red].setRect(1000, 130, 250, 50);
    buttons[ButtonType::red].setFont(GraphicsManager::instance().get_button_font());
    buttons[ButtonType::red].setText("Piros");
    buttons[ButtonType::red].setOnClick([](){
        if (!is_spinned && money > 0)
        {
            spin_timer = SDL_GetTicks();
            is_spinned = true;
            chosen_color = 1;
            roullette_winner = getRandomNumber(11) / 5;
        }
    });
    buttons[ButtonType::wheel].setRect(100, 10, 588, 682);
    buttons[ButtonType::wheel].setColor(255, 255, 255, 0, ColorRole::color_hover);
    buttons[ButtonType::wheel].setColor(255, 255, 255, 0, ColorRole::color_pressed);
    buttons[ButtonType::green].setRect(1000, 230, 250, 50);
    buttons[ButtonType::green].setFont(GraphicsManager::instance().get_button_font());
    buttons[ButtonType::green].setText("Zöld");
    buttons[ButtonType::green].setOnClick([](){
        if (!is_spinned && money > 0)
        {
            spin_timer = SDL_GetTicks();
            is_spinned = true;
            chosen_color = 2;
            roullette_winner = getRandomNumber(11) / 5;
        }
    });
    buttons[ButtonType::back].setRect(1180, 680, 100, 40);
    buttons[ButtonType::back].setFont(GraphicsManager::instance().get_button_font());
    buttons[ButtonType::back].setText("Vissza");
    buttons[ButtonType::back].setOnClick([](){
        if (!is_spinned)
        {
            setUpMainMenu();
            buttons[ButtonType::wheel].hide();
            buttons[ButtonType::black].hide();
            buttons[ButtonType::red].hide();
            buttons[ButtonType::green].hide();
            buttons[ButtonType::back].hide();
        }
    });
}

void setUpDefinitions(int page)
{
    buttons[ButtonType::switch_left].setRect(40, 40, 36, 36);
    buttons[ButtonType::switch_left].setOnClick([](){
        current_page = 1;
        setUpDefinitions(current_page);
    });
    buttons[ButtonType::switch_right].setRect(1200, 40, 36, 36);
    buttons[ButtonType::switch_right].setOnClick([](){
        current_page = 2;
        setUpDefinitions(current_page);
    });

    for (int i = 0; i < config::DEFINITION_COUNT; i++)
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
    buttons[ButtonType::back].setRect(1180, 680, 100, 40);
    buttons[ButtonType::back].setFont(GraphicsManager::instance().get_button_font());
    buttons[ButtonType::back].setText("Vissza");
    buttons[ButtonType::back].setOnClick([](){
        for (int i = 0; i < config::DEFINITION_COUNT; i++) def_buttons[i].hide();
        buttons[ButtonType::switch_left].setRect(0, 0, 0, 0);
        buttons[ButtonType::switch_right].setRect(0, 0, 0, 0);
        setUpMainMenu();
        buttons[ButtonType::back].hide();
    });
}

void drawLineOnScreen(std::string& text, TTF_Font* font, SDL_Color text_color, int posx, int posy)
{
    if (text.empty()) return;
    if (text == config::HIDDEN_TEXT)
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
    for (int i = 0; i < config::PROGRAMS_COUNT+4; i++)
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
    buttons[ButtonType::show_colors].setRect(1180, 630, 100, 40);
    buttons[ButtonType::show_colors].setFont(GraphicsManager::instance().get_button_font());
    buttons[ButtonType::show_colors].setText("Szinek");
    buttons[ButtonType::show_colors].setOnClick([](){
        if (!color_panel_on)
            setUpColorButtons();  
        else
            hideColorButtons();
    });
}

void setDifficultyNames()
{
    prog_buttons[0].setText("Easy");
    prog_buttons[7].setText("Medium");
    prog_buttons[14].setText("Hacker");
    prog_buttons[21].setText("String");
}

void setUpBackButtonInCoding()
{
    buttons[ButtonType::back].setOnClick([](){
        hideColorButtons();
        buttons[ButtonType::show_colors].hide();
        buttons[ButtonType::hint].hide();
        rendered_code_lines = {};
        setUpPrograms();
    });
}

void setUpHintButton()
{
    buttons[ButtonType::hint].setRect(1180, 580, 100, 40);
    buttons[ButtonType::hint].setFont(GraphicsManager::instance().get_button_font());
    buttons[ButtonType::hint].setText("Mutasd");
    buttons[ButtonType::hint].setOnClick([](){
        SDL_StopTextInput(GraphicsManager::instance().get_window());
        const char* p = hidden_line.first.c_str();
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Megoldás", p, GraphicsManager::instance().get_window());
        rendered_code_lines[hidden_line.second] = hidden_line.first;
        buttons[ButtonType::hint].hide();
    });
}

void setUpProgramButton(size_t index)
{
    std::string program_text = std::to_string(index - index/7);
    prog_buttons[index].setText(program_text);
    prog_buttons[index].setOnClick([index](){
        SDL_StartTextInput(GraphicsManager::instance().get_window());
        setUpShowColorsButton();
        getProgramRequest(index - index/7);
        readProgram(index - index/7);
        hidePrograms();     
        setUpBackButtonInCoding();
        setUpHintButton();
    });
}

void setUpBackButtonInPrograms()
{
    buttons[ButtonType::back].setRect(1180, 680, 100, 40);
    buttons[ButtonType::back].setFont(GraphicsManager::instance().get_button_font());
    buttons[ButtonType::back].setText("Vissza");
    buttons[ButtonType::back].setOnClick([](){
        SDL_StopTextInput(GraphicsManager::instance().get_window());
        setUpMainMenu();
        hidePrograms();
        buttons[ButtonType::back].hide();
    });
}

void setUpPrograms()
{
    setDifficultyNames();
    for (int i = 0; i < config::PROGRAMS_COUNT+4; i++)
    {
        prog_buttons[i].setRect(100 + 300 * (i/7), 100 + 75 * (i%7), 200, 40);
        prog_buttons[i].setFont(GraphicsManager::instance().get_button_font());
        if (i%7 != 0)
            setUpProgramButton(i);
    }
    setUpBackButtonInPrograms();
}

void setAllButtonTextures()
{
    for (int i = 0; i < config::BUTTON_COUNT; i++)
        buttons[i].setTexture(GraphicsManager::instance().get_button_texture());
    buttons[ButtonType::logo].setTexture(GraphicsManager::instance().get_logo_texture());
    buttons[ButtonType::switch_right].setTexture(GraphicsManager::instance().get_switch_texture());
    buttons[ButtonType::switch_left].setTexture(GraphicsManager::instance().get_switch_texture());
    buttons[ButtonType::switch_left].setFlipMode(SDL_FLIP_HORIZONTAL);
    buttons[ButtonType::wheel].setTexture(GraphicsManager::instance().get_wheel_texture());
    for (int i = 0; i < config::DEFINITION_COUNT; i++)
        def_buttons[i].setTexture(GraphicsManager::instance().get_button_texture());
    for (int i = 0; i < config::PROGRAMS_COUNT + 4; i++)
        prog_buttons[i].setTexture(GraphicsManager::instance().get_button_texture());
}

bool InitializeAll()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("SDL inicializalas hiba: %s", SDL_GetError());
        return 0;
    }
    if (!TTF_Init())
    {
        SDL_Log("Nem sikerult inicializalni a fontot: %s", SDL_GetError());
        return 0;
    }
    if (!GraphicsManager::instance().initialize(window_size::WIDTH, window_size::HEIGHT))
    {
        return 0;
    }
    return 1;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    if (!InitializeAll())
        return SDL_APP_FAILURE;
    SDL_SetAppMetadata("Tanulas", "1.0", "tanulas");
    SDL_SetWindowResizable(GraphicsManager::instance().get_window(), 0);
    SDL_SetRenderDrawBlendMode(GraphicsManager::instance().get_renderer(), SDL_BLENDMODE_BLEND);
    setAllButtonTextures();
    setUpMainMenu();
    getTitlesAndContents();  
    return SDL_APP_CONTINUE;
}

void handleAllButtons(SDL_Event* event)
{
    for (int i = 0; i < config::BUTTON_COUNT; i++)
        buttons[i].handleEvent(*event);
    for (int i = 0; i < config::DEFINITION_COUNT; i++)
        def_buttons[i].handleEvent(*event);
    for (int i = 0; i < config::PROGRAMS_COUNT + 4; i++)
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
        if (rendered_code_lines[hidden_line.second] == config::HIDDEN_TEXT)
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
                if (rendered_code_lines[hidden_line.second] == config::HIDDEN_TEXT)
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
            buttons[ButtonType::hint].hide();
        }
    }
    handleAllButtons(event);
    return SDL_APP_CONTINUE;
}

void drawAllButtons()
{
    for (int i = 0; i < config::BUTTON_COUNT; i++)
        buttons[i].draw(GraphicsManager::instance().get_renderer());
    for (int i = 0; i < config::DEFINITION_COUNT; i++)
        def_buttons[i].draw(GraphicsManager::instance().get_renderer());
    for (int i = 0; i < config::PROGRAMS_COUNT+4; i++)
        prog_buttons[i].draw(GraphicsManager::instance().get_renderer());
    for (int i = 0; i < colors::ROWS; i++)
        for (int j = 0; j < colors::COLS; j++)
            color_buttons[i][j].draw(GraphicsManager::instance().get_renderer());
}

void renderAllButtons()
{
    for (int i = 0; i < config::BUTTON_COUNT-1; i++)
        buttons[i].renderButton(GraphicsManager::instance().get_renderer());
    for (int i = 0; i < config::DEFINITION_COUNT; i++)
        def_buttons[i].renderButton(GraphicsManager::instance().get_renderer());
    for (int i = 0; i < config::PROGRAMS_COUNT+4; i++)
        prog_buttons[i].renderButton(GraphicsManager::instance().get_renderer());
    if (is_spinned)
    {
        SDL_FRect source{0, 0, 588, 588};
        SDL_FRect dest{100, 10, 588, 588}; 
        const Uint64 now = SDL_GetTicks() - spin_timer;
        SDL_FPoint center = {294, 294};
        double angle = (now % 3000) / (3000.0f / 360.0f);
        buttons[ButtonType::wheel].renderButton(GraphicsManager::instance().get_renderer(),
                                                &source, &dest, angle, &center);
        if (now > 4300 + 90 * roullette_winner)
        {
            std::string outcome;
            if (chosen_color == roullette_winner)
            {
                outcome = "Nyertél!";
                if (chosen_color == 2) money += 10;
                else money++;
            }
            else
            {
                outcome = "Vesztettél!";
                money--;
            }
            money_line = "Pénz: " + std::to_string(money);
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Rullett", outcome.c_str(), GraphicsManager::instance().get_window());
            is_spinned = false;
        }
        SDL_FRect arrow_source = {250, 600, 81, 81};
        SDL_FRect arrow_dest = {350, 610, 81, 81};
        buttons[ButtonType::wheel].renderButton(GraphicsManager::instance().get_renderer(),
                                                &arrow_source, &arrow_dest);
    }
    else buttons[ButtonType::wheel].renderButton(GraphicsManager::instance().get_renderer());
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    Uint64 current_time = SDL_GetTicks();
    if (current_time - money_timer > 300000)
    {
        money++;
        money_timer = current_time;
        money_line = "Pénz: " + std::to_string(money);
    }
    SDL_SetRenderDrawColor(GraphicsManager::instance().get_renderer(), 33, 33, 33, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(GraphicsManager::instance().get_renderer());
    SDL_RenderTexture(GraphicsManager::instance().get_renderer(),
                      GraphicsManager::instance().get_background_texture(), nullptr, nullptr);
    renderAllButtons();
    drawAllButtons();
    for (int i = 0; i < 40; i++)
        drawLineOnScreen(rendered_code_lines[i], GraphicsManager::instance().get_code_font(), code_color, 0, 40*i);
    drawLineOnScreen(money_line, GraphicsManager::instance().get_code_font(), code_color, 10, 10);
    SDL_RenderPresent(GraphicsManager::instance().get_renderer());
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result){
    SDL_StopTextInput(GraphicsManager::instance().get_window());
}