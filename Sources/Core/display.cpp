#include <GL/glew.h>
#include <iostream>
#include "Core/display.h"

void Display::Construct(const std::string &title)
{
    std::cout << "Constructed" << std::endl;
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    if(window == NULL) LOG(SDL_GetError());
    glContext = SDL_GL_CreateContext(window);


#ifdef _WIN32
    SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
#endif
    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // glEnable(GL_TEXTURE_CUBE_MAP);

    GLenum status = glewInit();
    if (status != GLEW_OK)
    {
        std::cerr << "Glew failed to initialise" << std::endl;
        LOG(glewGetErrorString(status));
    }
}

void Display::Destruct()
{
    std::cout << "Destructed" << std::endl;

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Display::Update()
{
    SDL_GL_SwapWindow(window);
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            isClosed = true;
    }
}

bool Display::IsClosed()
{
    return isClosed;
}

void Display::Clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f)
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//Checks if the LBM is pressed and get the coords of the cursour
void Display::FetchMouseState(int &x, int &y, bool &isLbmPressed)
{
    Uint32 buttons = SDL_GetMouseState(&x, &y);
    isLbmPressed = buttons & SDL_BUTTON_LMASK;
}

bool Display::ReadKeyboardState(SDL_Scancode code)
{
    return keys[code];
}

SDL_Scancode *Display::KeyScancodeMap()
{
    return keyScancodeMap;
}