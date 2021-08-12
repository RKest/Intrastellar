#include "Core/controler.h"
#include <iostream>
#include "glm/gtx/string_cast.hpp"

void Controler::CaptureMouseMovement()
{
    int xCoord, yCoord;
    display.FetchMouseState(xCoord, yCoord);
    ft halfDisplayWidth = (ft)display.width / 2;
    ft halfDisplayHeight = (ft)display.height / 2;

    ft xPcCoord = (xCoord - halfDisplayWidth) / halfDisplayWidth;
    ft yPcCoord = -(yCoord - halfDisplayHeight) / halfDisplayHeight;

    glm::vec2 mousePos = glm::inverse(camera.ViewProjection()) * glm::vec4(xPcCoord, yPcCoord, 0.0f, 0.0f);
    glm::vec2 pcPos = transform.Pos();
    glm::vec2 vecToMouse = glm::normalize(pcPos - mousePos);

    ft angle = -glm::atan(glm::dot({-1.0, 0.0}, vecToMouse), det({-1.0, 0.0}, vecToMouse));
    transform.SetRotAngle(angle);
}

void Controler::CaptureKeyboardPresses(bool &isPcAlive)
{
    if (isPcAlive)
    {
        db movementAmount = timer.Scale(defaultMovementAmount);
        if (display.ReadKeyboardState(codes[display.W]))
            transform.Pos().y += movementAmount;
        if (display.ReadKeyboardState(codes[display.S]))
            transform.Pos().y -= movementAmount;
        if (display.ReadKeyboardState(codes[display.A]))
            transform.Pos().x += movementAmount;
        if (display.ReadKeyboardState(codes[display.D]))
            transform.Pos().x -= movementAmount;
    }
    if (display.ReadKeyboardState(codes[display.SPACE]))
        isPcAlive = true;
}

ft Controler::det(const glm::vec2 &vec1, const glm::vec2 &vec2)
{
    return vec1.x * vec2.y - vec1.y * vec2.x;
}