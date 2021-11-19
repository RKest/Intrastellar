#include "Core/controler.h"
#include <iostream>
#include "glm/gtx/string_cast.hpp"

void Controler::CaptureMouseMovement()
{
    if(IDisplay::I.ReadKeyboardState(codes[IDisplay::I.TAB]))
        return;
    int xCoord, yCoord;
    IDisplay::I.FetchMouseState(xCoord, yCoord);
    constexpr const ft halfDisplayWidth   = static_cast<ft>(SCREEN_WIDTH) / 2.0f;
    constexpr const ft halfDisplayHeight  = static_cast<ft>(SCREEN_HEIGHT) / 2.0f;

    const ft xPcCoord = (decl_cast(halfDisplayWidth, xCoord) - halfDisplayWidth) / halfDisplayWidth;
    const ft yPcCoord = -(decl_cast(halfDisplayHeight, yCoord) - halfDisplayHeight) / halfDisplayHeight;

    const glm::mat4 cameraProjectionInverse = glm::inverse(Camera::ViewProjection()); 
    const glm::vec2 mousePos = glm::vec2(cameraProjectionInverse * glm::vec4(xPcCoord, yPcCoord, 0.0f, 0.0f));
    const glm::vec2 pcPos = glm::vec2(0);
    const glm::vec2 vecToMouse = glm::normalize(pcPos - mousePos);

    const ft angle = PI / 2.0f - glm::atan(glm::dot({-1.0, 0.0}, vecToMouse), helpers::det({-1.0, 0.0}, vecToMouse));
    transform.SetRotAngle(angle);
}

void Controler::CaptureKeyboardPresses(bool &isPcAlive)
{
    if (isPcAlive)
    {
        const ft movementAmount = decl_cast(movementAmount, Timer::Scale(defaultMovementAmount));
        if (IDisplay::I.ReadKeyboardState(codes[IDisplay::I.W]))
            transform.Pos().y += movementAmount,
              Camera::Pos().y += movementAmount;
        if (IDisplay::I.ReadKeyboardState(codes[IDisplay::I.S]))
            transform.Pos().y -= movementAmount,
              Camera::Pos().y -= movementAmount;
        if (IDisplay::I.ReadKeyboardState(codes[IDisplay::I.A]))
            transform.Pos().x += movementAmount,
              Camera::Pos().x += movementAmount;
        if (IDisplay::I.ReadKeyboardState(codes[IDisplay::I.D]))
            transform.Pos().x -= movementAmount,
              Camera::Pos().x -= movementAmount;
    }
    if (IDisplay::I.ReadKeyboardState(codes[IDisplay::I.SPACE]))
        isPcAlive = true;
}
