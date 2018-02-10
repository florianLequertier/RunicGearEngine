#include "WindowContext.hpp"

WindowContext::WindowContext()
{
    if (!glfwInit())
    {
        std::cout<<"Error : can't init glfw !"<<std::endl;
    }
}

WindowContext::~WindowContext()
{
    glfwTerminate();
}

void WindowContext::PoolEvents() const
{
    glfwPollEvents();
}
