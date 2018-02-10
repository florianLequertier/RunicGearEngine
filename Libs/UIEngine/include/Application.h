#pragma once

#include <iostream>

#include "glew.h"
#include "GLFW/glfw3.h"

class Application
{
protected:
	GLFWwindow* window;
	int viewportWidth;
	int viewportHeight;
	float viewportRatio;

public:
	Application()
	{
		initWindow();
		initGlew();
	}

	~Application()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void run();

	virtual void init() = 0;
	virtual void update() = 0;
	virtual void render() = 0;

	static void s_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Application* app = (Application*)glfwGetWindowUserPointer(window);
		app->keyCallback(window, key, scancode, action, mods);;
	}
	static void s_characterCallback(GLFWwindow* window, unsigned int codepoint)
	{
		Application* app = (Application*)glfwGetWindowUserPointer(window);
		app->characterCallback(window, codepoint);
	}
	static void s_cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
	{
		Application* app = (Application*)glfwGetWindowUserPointer(window);
		app->cursorPositionCallback(window, xpos, ypos);
	}
	static void s_mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		Application* app = (Application*)glfwGetWindowUserPointer(window);
		app->mouseButtonCallback(window, button, action, mods);
	}

	virtual void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) = 0;
	virtual void characterCallback(GLFWwindow* window, unsigned int codepoint) = 0;
	virtual void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) = 0;
	virtual void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) = 0;

private:
	int initWindow()
	{
		/* Initialize the library */
		if (!glfwInit())
			return -1;

		/* Create a windowed mode window and its OpenGL context */
		window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
		if (!window)
		{
			glfwTerminate();
			return -1;
		}

		/* Make the window's context current */
		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);

		glfwSetWindowUserPointer(window, this);

		glfwSetKeyCallback(window, s_keyCallback);
		glfwSetCharCallback(window, s_characterCallback);
		glfwSetCursorPosCallback(window, s_cursorPositionCallback);
		glfwSetMouseButtonCallback(window, s_mouseButtonCallback);

		glfwGetFramebufferSize(window, &viewportWidth, &viewportHeight);
		viewportRatio = viewportWidth / (float)viewportHeight;
	}

	void initGlew()
	{
		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			/* Problem: glewInit failed, something is seriously wrong. */
			std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		}
		std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << std::endl;
	}

};

void Application::run()
{
	while (!glfwWindowShouldClose(window))
	{
		update();

		render();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);
		/* Poll for and process events */
		glfwPollEvents();
	}
}

