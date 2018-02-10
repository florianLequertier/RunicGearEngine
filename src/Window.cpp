#include "Window.hpp"

Window::Window()
{
    m_window = glfwCreateWindow(640, 480, "My Window", NULL, NULL);
    if (!m_window)
    {
        std::cout<<"Error : can't create glfw window !"<<std::endl;
    }
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
}

void Window::SetAsCurrentContext() const
{
    glfwMakeContextCurrent(m_window);
}

bool Window::ShouldClose() const
{
    return (bool)glfwWindowShouldClose(m_window);
}

void Window::SwapBuffers() const
{
    glfwSwapBuffers(m_window);
}