#include "Core/transform.h"
#include "Core/display.h"
#include "Core/camera.h"
#include <iostream>

#include <SDL2/SDL_scancode.h>

class Controler
{
public:
    Controler(Display &display, Camera &camera, Transform &transform) : display(display), camera(camera), transform(transform){};
    void CaptureMouseMovement();
    void CaptureKeyboardPresses(bool &isPcAlive);

protected:
private:
    Display &display;
    Camera &camera;
    Transform &transform;

    SDL_Scancode *codes = display.KeyScancodeMap();
    float defaultMovementAmount = 0.01f;

    ft Det(const glm::vec2 &vec1, const glm::vec2 &vec2);
};
