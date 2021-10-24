#pragma once
#include "_config.h"
#include "Core/transform.h"
#include "Core/display.h"
#include "Core/camera.h"
#include "Core/timer.h"
#include "Core/helpers.h"
#include <iostream>

#include <SDL2/SDL_scancode.h>

class Controler
{
public:
    Controler(Display &display, Camera &camera, Timer &timer, Transform &transform) 
    : display(display), camera(camera), timer(timer), transform(transform) {};
    void CaptureMouseMovement();
    void CaptureKeyboardPresses(bool &isPcAlive);
    inline bool IsTabPressed() { return display.ReadKeyboardState(codes[display.TAB]); }

protected:
private:
    Display &display;
    Camera &camera;
    Timer &timer;
    Transform &transform;

    SDL_Scancode *codes = display.KeyScancodeMap();
    db defaultMovementAmount = 0.02f;

};