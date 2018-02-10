#pragma once

#include "Widget.h"

class EmptyWidget : public Widget
{
public:
	EmptyWidget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> program)
		: Widget(uiengine, shape, program)
	{}
};
