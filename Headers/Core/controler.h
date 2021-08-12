#include "_config.h"
#include "Core/transform.h"
#include "Core/display.h"
#include "Core/camera.h"
#include "Core/timer.h"
#include <iostream>

#include <SDL2/SDL_scancode.h>

class Controler
{
public:
    Controler(Display &display, Camera &camera, Timer &timer, Transform &transform) 
    : display(display), camera(camera), transform(transform), timer(timer) {};
    void CaptureMouseMovement();
    void CaptureKeyboardPresses(bool &isPcAlive);

protected:
private:
    Display &display;
    Camera &camera;
    Timer &timer;
    Transform &transform;

    SDL_Scancode *codes = display.KeyScancodeMap();
    db defaultMovementAmount = 0.001f;

    ft det(const glm::vec2 &vec1, const glm::vec2 &vec2);
};
