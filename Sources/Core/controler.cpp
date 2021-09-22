#include "Core/controler.h"
#include <iostream>
#include "glm/gtx/string_cast.hpp"

void Controler::CaptureMouseMovement()
{
    int xCoord, yCoord;
    display.FetchMouseState(xCoord, yCoord);
    const ft halfDisplayWidth = (ft)display.width / 2;
    const ft halfDisplayHeight = (ft)display.height / 2;

    const ft xPcCoord = (decl_cast(halfDisplayWidth, xCoord) - halfDisplayWidth) / halfDisplayWidth;
    const ft yPcCoord = -(decl_cast(halfDisplayHeight, yCoord) - halfDisplayHeight) / halfDisplayHeight;

    const glm::mat4 cameraProjectionInverse = glm::inverse(camera.ViewProjection()); 
    const glm::vec2 mousePos = glm::vec2(cameraProjectionInverse * glm::vec4(xPcCoord, yPcCoord, 0.0f, 0.0f));
    const glm::vec2 pcPos = glm::vec2(0);
    const glm::vec2 vecToMouse = glm::normalize(pcPos - mousePos);

    const ft angle = -glm::atan(glm::dot({-1.0, 0.0}, vecToMouse), helpers::det({-1.0, 0.0}, vecToMouse));
    transform.SetRotAngle(angle);
}

void Controler::CaptureKeyboardPresses(bool &isPcAlive)
{
    if (isPcAlive)
    {
        const ft movementAmount = decl_cast(movementAmount, timer.Scale(defaultMovementAmount));
        if (display.ReadKeyboardState(codes[display.W]))
            transform.Pos().y += movementAmount,
            camera.Pos().y += movementAmount;
        if (display.ReadKeyboardState(codes[display.S]))
            transform.Pos().y -= movementAmount,
            camera.Pos().y -= movementAmount;
        if (display.ReadKeyboardState(codes[display.A]))
            transform.Pos().x += movementAmount,
            camera.Pos().x += movementAmount;
        if (display.ReadKeyboardState(codes[display.D]))
            transform.Pos().x -= movementAmount,
            camera.Pos().x -= movementAmount;
    }
    if (display.ReadKeyboardState(codes[display.SPACE]))
        isPcAlive = true;
}
