#pragma once
#include "_config.h"
#include <string>
#include <SDL2/SDL.h>

class Display
{
public:
    Display(int _width, int _height, const std::string &title);

    void Clear(float r, float g, float b, float a);
    void Update();
    void FetchMouseState(int &x, int &y, bool &isLbmPressed = _dummy_bool);
    bool ReadKeyboardState(SDL_Scancode code);

    enum { W, A, S, D, LCTRL, SPACE, TAB};
    SDL_Scancode *KeyScancodeMap();

    bool IsClosed();
    float Aspect();
    virtual ~Display();

    int width, height;
protected:
private:
    SDL_Window *window;
    SDL_GLContext glContext;
    bool isClosed;
    float aspectRatio;
    float halfWidth, halfHeight;
    int mouseX = 0, mouseY = 0;

    int mouseXOffset = 0, mouseYOffset = 0;
    bool mouseEventFilter = false;

    SDL_Scancode keyScancodeMap[7] = { SDL_SCANCODE_W, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_LCTRL, SDL_SCANCODE_SPACE, SDL_SCANCODE_TAB };

    const Uint8 *keys = SDL_GetKeyboardState(NULL);

};
