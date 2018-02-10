#pragma once

#include "GLFW/glfw3.h"
#include <iostream>

class WindowContext
{
private:

public:
    WindowContext();
    ~WindowContext();

    void PoolEvents() const;
};