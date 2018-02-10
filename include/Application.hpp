#pragma once

#include "glad/glad.h"
#include "Window.hpp"

class Application
{
private:
    WindowContext m_windowContext;
    Window m_mainWindow;

public:
    Application();
    ~Application();
    int Run();
};