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
    Controler(Transform &transform) 
    : transform(transform) {};
    void CaptureMouseMovement();
    void CaptureKeyboardPresses(bool &isPcAlive);
    inline bool IsTabPressed() { return Display::ReadKeyboardState(codes[Display::TAB]); }

protected:
private:
    Transform &transform;

    SDL_Scancode *codes = Display::KeyScancodeMap();
    db defaultMovementAmount = 0.02f;

};