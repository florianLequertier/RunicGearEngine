#pragma once

#include <functional>
#include "glm/glm.hpp"
#include "OpenglUtils.h"

class UIEngine;

class UIItem
{
protected:
	bool m_hovered;
	bool m_selected;
	bool m_pressed;
	UIEngine* m_uiEngine;

public:
//	std::function<bool(glm::vec2)> onMouseMove;
//	std::function<bool(int)> onMouseButtonPressed;
//	std::function<bool(int)> onMouseButtonReleased;
//	std::function<bool()> onMouseEnter;
//	std::function<bool()> onMouseLeave;
//	std::function<bool(int key)> onKeyPressed;
//	std::function<bool(int key)> onKeyReleased;
//	std::function<bool(unsigned int codepoint)> onCharacter;

	std::function<void(const glm::vec2&)> dragBeginCallback;
	std::function<void(const glm::vec2&)> dragCallback;
	std::function<void(const glm::vec2&)> dragEndCallback;
	std::function<bool(const glm::vec2&)> dropCallback;
	std::function<void()> dragEnterCallback;
	std::function<bool(const glm::vec2&)> dragOverCallback;
	std::function<void()> dragLeaveCallback;

public:
	UIItem(UIEngine* uiEngine);
	virtual ~UIItem();

	//virtual const glm::vec2& getPositionInViewport() const
	//{
	//	return m_posInViewport;
	//}
	
	//virtual void computePositionInViewportRecur() = 0;
	//virtual void computePositionInViewport() = 0;
	//virtual const glm::vec2& getPosition() const = 0;
	//virtual const glm::vec2& getSize() const = 0;

	virtual void draw(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const = 0;

	virtual bool isMouseHovering(const glm::vec2& cursor) const = 0;
	bool getIsHovered() const
	{
		return m_hovered;
	}
	void setIsHovered(bool hovered)
	{
		m_hovered = hovered;
	}
	virtual bool acceptLayer() const
	{
		return false;
	}
	bool getIsSelected() const
	{
		return m_selected;
	}
	void setIsSelected(bool isSelected)
	{
		m_selected = isSelected;
		if (m_selected)
			onItemSelected();
		else
			onItemUnselected();
	}
	bool getIsPressed() const
	{
		return m_pressed;
	}
	void setIsPressed(bool pressed)
	{
		m_pressed = pressed;
	}

	virtual bool handleMouseMove(const glm::vec2 mousePos);
	virtual bool handleMouseButtonPressed(int button, const glm::vec2& mousePos);
	virtual bool handleMouseButtonReleased(int button, const glm::vec2& mousePos);
	virtual bool handleKeyPressed(int key);
	virtual bool handleKeyReleased(int key);
	virtual bool handleCharacter(unsigned int codepoint);
	virtual bool handleDragOver(const glm::vec2& mousePos);
	virtual bool handleDrop(const glm::vec2& mousePos);
	// only append on the dragged item, don't bubble

	virtual bool onMouseMove(const glm::vec2& mousePos) { return false; }
	virtual bool onMouseButtonPressed(int button, const glm::vec2& mousePos) { return false; }
	virtual bool onMouseButtonReleased(int button, const glm::vec2& mousePos) { return false; }
	virtual bool onMouseEnter() { return false; }
	virtual bool onMouseLeave() { return false; }
	virtual bool onKeyPressed(int key) { return false; }
	virtual bool onKeyReleased(int key) { return false; }
	virtual bool onCharacter(unsigned int codepoint) { return false; }
	virtual bool onDrop(const glm::vec2& mousePos) { if (dropCallback) { return dropCallback(mousePos); } else { return false; } }
	virtual void onDragEnter() { if (dragEnterCallback) { dragEnterCallback(); } }
	virtual bool onDragOver(const glm::vec2& mousePos) { if (dragOverCallback) { return dragOverCallback(mousePos); } else { return false; } }
	virtual void onDragLeave() { if (dragLeaveCallback) { dragLeaveCallback(); } }
	////////////
	virtual void onDragBegin(const glm::vec2& mousePos) { if (dragBeginCallback) { dragBeginCallback(mousePos); } }
	virtual bool onDrag(const glm::vec2& mousePos) { if (dragCallback) { dragCallback(mousePos); } }
	virtual void onDragEnd(const glm::vec2& mousePos) { if (dragEndCallback) { dragEndCallback(mousePos); } }

	virtual void onItemSelected() {}
	virtual void onItemUnselected() {}
};