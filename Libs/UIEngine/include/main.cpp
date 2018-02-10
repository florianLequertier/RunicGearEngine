#include "glew.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <map>
#include <functional>
#include "glm/glm.hpp"

#include "Widget.h"
#include "WidgetLayer.h"
#include "EmptyWidget.h"
#include "Application.h"
#include "UIEngine.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class MyApplication final : public Application
{
private:
	// test opengl
	VAO vao;
	ShaderProgram program;
	/////

	UIEngine uiengine;
	glm::vec2 cursorPos;

	// resources : 
	std::shared_ptr<Texture> m_defaultTexture;

public:
	void init() override;
	void update() override;
	void render() override;

	// input
	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) override;
	void characterCallback(GLFWwindow* window, unsigned int codepoint) override;
	void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) override;
	void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) override;
};

void MyApplication::init()
{
	// test opengl
	vao.setDatas(vertices, indices);
	program.load("resources/shaders/UIWidget.vert", "resources/shaders/UIWidget.frag");
	//////

	// initialize resources
	m_defaultTexture = Texture::load_RGB_image("resources/images/default.jpg");
	//m_defaultFont = std::make_shared <Font>();
	//m_defaultFont->load()

	uiengine.getFontFactory().loadFont("resources/fonts/OpenSans-Regular.ttf", "default", 48);
	uiengine.getFontFactory().setFontAsDefault("default", 48);

	// set viewport size
	uiengine.getRootViewportWidget()->setViewport(glm::vec2(0, 0), glm::vec2(viewportWidth, viewportHeight));

	// We create our widget
	{
		// We instantiate the widgets with the help of the factory function of the UIEngine.
		auto baseLayer = uiengine.instantiateLayer("Raw");

		// Add base layer childs
		{
			auto firstWidget = uiengine.instantiateWidget("EmptyWidget");
			auto firstWidgetLayer = uiengine.instantiateLayer("Raw");

			firstWidget->setLayer(firstWidgetLayer);
			// We add an horizontal layer to our widget and we had two child widget to this layer.
			RawSlot* firstWidgetSlot = baseLayer->addSlotAs<RawSlot>(firstWidget);

			firstWidgetSlot->setSize(glm::vec2(400, 150));
			firstWidgetSlot->setPosition(glm::vec2(50, 0));
			firstWidget->setTint(glm::vec4(0, 0, 1, 1));

			// Add first widget childs
			{
				auto textWidget = uiengine.instantiateWidgetAs<TextWidget>("TextWidget");
				auto textInputWidget = uiengine.instantiateWidgetAs<TextInputWidget>("TextInputWidget");
				auto buttonWidget = uiengine.instantiateWidgetAs<ButtonWidget>("ButtonWidget");
				RawSlot* textWidgetSlot = static_cast<RawSlot*>(firstWidgetLayer->addSlot(textWidget));
				RawSlot* textInputWidgetSlot = static_cast<RawSlot*>(firstWidgetLayer->addSlot(textInputWidget));
				RawSlot* buttonWidgetSlot = static_cast<RawSlot*>(firstWidgetLayer->addSlot(buttonWidget));

				// Note : 
				// layers has ownership over slots which have themselve the ownbership over their handled widget
				// when a layer is destroyed, all of these childs(slots/widgets) are destroy no other shared_ptr points to them.
				// if you want to replace a layer without destroying existing child widgets, create the new layer, then add all the widgets to this new layer
				// after that, you can safely destroy the old layer. The widgets remains alive because the new layer reference them.

				textWidgetSlot->setSize(glm::vec2(50, 50));
				textWidgetSlot->setPosition(glm::vec2(0, 0));
				textWidget->setTint(glm::vec4(0, 1, 0, 1));
				textWidget->setFont(uiengine.getFontFactory().getFont("default"));
				textWidget->setText("ab");

				textInputWidgetSlot->setSize(glm::vec2(50, 50));
				textInputWidgetSlot->setPosition(glm::vec2(0, 50));
				textInputWidget->setTint(glm::vec4(0, 1, 0, 1));
				textInputWidget->setFont(uiengine.getFontFactory().getFont("default"));
				textInputWidget->setText("ab");

				buttonWidgetSlot->setSize(glm::vec2(100, 50));
				buttonWidgetSlot->setPosition(glm::vec2(0, 100));
				buttonWidget->setTint(glm::vec4(0, 1, 0, 1));
				buttonWidget->setStyle(ButtonStyle(glm::vec4(0, 1, 0, 1), WidgetPadding(), glm::vec4(1, 0, 0, 1), WidgetPadding(10, 10, 10, 10), glm::vec4(0, 0, 1, 1), WidgetPadding()));
				buttonWidget->onClicked = [](int button, const glm::vec2& mousePos) { std::cout << "Button clicked !!!" << std::endl; };

				// Add button childs
				{
					auto buttonLayout = uiengine.instantiateLayer("Canvas");
					buttonWidget->setLayer(buttonLayout);

					auto buttonImage = uiengine.instantiateWidgetAs<ImageWidget>("ImageWidget");

					auto buttonSlot = buttonLayout->addSlotAs<CanvasSlot>(buttonImage);

					buttonSlot->setSize(glm::vec2(100, 100));
					buttonSlot->getAnchor().anchorPosition = glm::vec2(0, 0);
					buttonImage->setTint(glm::vec4(0, 1, 0, 1));
					buttonImage->setTexture(m_defaultTexture);
				}
			}
		}
		

		// We had a callback reacting to the "mouse button pressed" input event on the firstSubWidget02
		//firstWidget->onMouseButtonPressed = [](int button) { std::cout << " button pressed event on first widget !!! " << std::endl; return true; };

		// Once the widget has been created, we had it to the engine which will take care of rendering it and handling its inputs
		uiengine.getRootViewportWidget()->addLayer(baseLayer, 0);

		// other callbacks : 
		//firstWidgetChild01->onMouseButtonPressed = [](int button) { std::cout << " button pressed event on first widget child 01 !!! " << std::endl; return true; };
	}
	///

	// We create our widget
	{
		auto baseLayer = uiengine.instantiateLayer("Raw");
		uiengine.getRootViewportWidget()->addLayer(baseLayer, 1);
		{
			auto firstWidget = uiengine.instantiateWidget("EmptyWidget");
			RawSlot* firstWidgetSlot = baseLayer->addSlotAs<RawSlot>(firstWidget);

			firstWidgetSlot->setSize(glm::vec2(400, 150));
			firstWidgetSlot->setPosition(glm::vec2(50, 200));
			firstWidget->setTint(glm::vec4(0, 0, 1, 1));

			// childs
			firstWidget->setLayer(uiengine.instantiateLayer("HorizontalList"));
			{
				auto firstWidgetChild01 = uiengine.instantiateWidget("EmptyWidget");
				ListSlot* slot01 = firstWidget->getLayer()->addSlotAs<ListSlot>(firstWidgetChild01);

				slot01->setFillX(false);
				slot01->setFillY(false);
				firstWidgetChild01->setTint(glm::vec4(0, 1, 0, 1));
			}

			{
				auto firstWidgetChild02 = uiengine.instantiateWidgetAs<ImageWidget>("ImageWidget");
				ListSlot* slot02 = firstWidget->getLayer()->addSlotAs<ListSlot>(firstWidgetChild02);

				slot02->setFillX(true);
				slot02->setFillY(true);
				firstWidgetChild02->setTint(glm::vec4(1, 1, 1, 1));
				firstWidgetChild02->setTexture(m_defaultTexture);
				firstWidgetChild02->setCornerRadius(0.05);
			}

			{
				auto firstWidgetChild03 = uiengine.instantiateWidget("EmptyWidget");
				ListSlot* slot03 = firstWidget->getLayer()->addSlotAs<ListSlot>(firstWidgetChild03);

				slot03->setFillX(false);
				slot03->setFillY(false);
				slot03->setSizeToContent(true);
				firstWidgetChild03->setTint(glm::vec4(0, 1, 1, 1));

				// childs
				firstWidgetChild03->setLayer(uiengine.instantiateLayer("HorizontalList"));
				{
					// Create a child for this widget
					auto firstWidgetChild03_child = uiengine.instantiateWidgetAs<TextWidget>("TextWidget");
					// Add the child to the layer
					auto* firstWidgetChild03_childSlot = firstWidgetChild03->getLayer()->addSlotAs<ListSlot>(firstWidgetChild03_child);

					// Setup the child
					firstWidgetChild03_child->setFont(uiengine.getFontFactory().getFont("default"));
					firstWidgetChild03_child->setText("Hello_aaaaaaaaaaa");
					firstWidgetChild03_childSlot->setSizeToContent(true);
				}
			}
		}
	}
	
	{
		auto layer = uiengine.instantiateLayer("Raw");
		auto dropDown = uiengine.instantiateWidgetAs<DropDownWidget>("DropDownWidget");
		auto dropDownSlot = layer->addSlotAs<RawSlot>(dropDown);
		dropDownSlot->setPosition(glm::vec2(50, 300));
		dropDownSlot->setSize(glm::vec2(200, 50));
		dropDown->setTint(glm::vec4(1, 0, 0, 1));

		dropDown->addDefaultItem("test01");
		dropDown->addDefaultItem("test02");
		dropDown->addDefaultItem("test03");
		dropDown->addDefaultItem("test04");

		uiengine.getRootViewportWidget()->addLayer(layer, 2);
	}

	//// We create our widget
	//{
	//	// We instantiate the widgets with the help of the factory function of the UIEngine.
	//	auto baseLayer = uiengine.instantiateLayer("Canvas");
	//	auto firstWidget = uiengine.instantiateWidget("EmptyWidget");
	//	auto horizontalLayer = uiengine.instantiateLayer("HorizontalList");
	//	auto firstSubWidget01 = uiengine.instantiateWidget("EmptyWidget");
	//	auto firstSubWidget02= uiengine.instantiateWidget("EmptyWidget");

	//	// Note : 
	//	// layers has ownership over slots which have themselve the ownbership over their handled widget
	//	// when a layer is destroyed, all of these childs(slots/widgets) are destroy no other shared_ptr points to them.
	//	// if you want to replace a layer without destroying existing child widgets, create the new layer, then add all the widgets to this new layer
	//	// after that, you can safely destroy the old layer. The widgets remains alive because the new layer reference them.

	//	firstWidget->setSize(glm::vec2(200, 300));

	//	// We add an horizontal layer to our widget and we had two child widget to this layer.
	//	baseLayer->addSlot(firstWidget);
	//	firstWidget->setLayer(horizontalLayer);
	//	horizontalLayer->addSlot(firstSubWidget01);
	//	horizontalLayer->addSlot(firstSubWidget02);

	//	// We had a callback reacting to the "mouse button pressed" input event on the firstSubWidget02
	//	firstSubWidget02->onMouseButtonPressed = [](int button) { std::cout << " button pressed event on first sub widget 02 !!! " << std::endl; return true; };

	//	// Once the widget has been created, we had it to the engine which will take care of rendering it and handling its inputs
	//	uiengine.getRootViewportWidget()->addLayer(baseLayer, 0);
	//}
	/////
}

void MyApplication::update()
{
	// game updates
}

void MyApplication::render()
{
	glViewport(0, 0, viewportWidth, viewportHeight);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	// test opengl
	//program.use();
	//vao.draw();

	uiengine.renderUI(glm::vec2(viewportWidth, viewportHeight));
}

void MyApplication::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	std::cout << "key detected : " << key << std::endl;

	if (action == GLFW_PRESS)
		uiengine.handleKeyPressed(key);
	else if (action == GLFW_RELEASE)
		uiengine.handleKeyReleased(key);
}

void MyApplication::characterCallback(GLFWwindow* window, unsigned int codepoint)
{
	std::cout << "character detected : " << (char)codepoint << std::endl;

	uiengine.handleCharacter(codepoint);
}

void MyApplication::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	cursorPos = glm::vec2(xpos, ypos);
	uiengine.handleMouseMove(cursorPos);
}

void MyApplication::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	std::cout << "mouse button detected : " << button << ", on position : "<< cursorPos.x << "," << cursorPos.y << std::endl;

	if(action == GLFW_PRESS)
		uiengine.handleMouseButtonPressed(button, cursorPos);
	else if(action == GLFW_RELEASE)
		uiengine.handleMouseButtonReleased(button, cursorPos);
}

int main()
{
	MyApplication app;
	app.init();
	app.run();
}