#pragma once

#include "WindowContext.hpp"

class Window
{
private:
    GLFWwindow* m_window;

public:
    Window();
    ~Window();

    void SetAsCurrentContext() const;
    bool ShouldClose() const;
    void SwapBuffers() const;
};