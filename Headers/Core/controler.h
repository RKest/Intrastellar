#pragma once
#include "_config.h"
#include "Core/transform.h"
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
    inline bool IsTabPressed() { return IDisplay::I.ReadKeyboardState(codes[IDisplay::I.TAB]); }

protected:
private:
    Transform &transform;

    SDL_Scancode *codes = IDisplay::I.KeyScancodeMap();
    db defaultMovementAmount = 0.02f;

};