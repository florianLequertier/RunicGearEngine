#include "Application.hpp"

Application::Application()
    // create window context
    : m_windowContext()
    // create the main window
    , m_mainWindow()
{
    // init opengl
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    // set main window as current context
    m_mainWindow.SetAsCurrentContext();
}

Application::~Application()
{
    // main window destroyed
    // window context destroyed
}

int Application::Run()
{
    // Run program
    while (!m_mainWindow.ShouldClose())
    {
        m_mainWindow.SwapBuffers();
        m_windowContext.PoolEvents(); 
    }

    return 0;
}