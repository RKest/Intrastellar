#pragma once
#include "_config.h"
#include <string>
#include <SDL2/SDL.h>

inline bool _dummy_bool;

class Display
{
public:
    Display(const std::string &title);
    static void Destruct();

    static void Clear(float r, float g, float b, float a);
    static void Update();
    static void FetchMouseState(int &x, int &y, bool &isLbmPressed = _dummy_bool);
    static bool ReadKeyboardState(SDL_Scancode code);

    enum { W, A, S, D, LCTRL, SPACE, TAB};
    static SDL_Scancode *KeyScancodeMap();

    static bool IsClosed();
protected:
private:
    friend class IDisplay;
    inline static SDL_Window *window;
    inline static SDL_GLContext glContext;
    inline static bool isClosed{};
    inline static float aspectRatio;
    inline static float halfWidth, halfHeight;
    inline static int mouseX = 0, mouseY = 0;

    inline static int mouseXOffset = 0, mouseYOffset = 0;
    inline static bool mouseEventFilter = false;

    inline static SDL_Scancode keyScancodeMap[7] = { SDL_SCANCODE_W, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_LCTRL, SDL_SCANCODE_SPACE, SDL_SCANCODE_TAB };

    inline static const Uint8 *keys = SDL_GetKeyboardState(NULL);

};

class IDisplay
{
public:
    inline static Display I{"Intrastellar"};
};
