#include "UIItem.h"
#include "UIEngine.h"

UIItem::UIItem(UIEngine* uiEngine)
	: m_uiEngine(uiEngine)
	, m_hovered(false)
	, m_selected(false)
{}

UIItem::~UIItem()
{
	// make sure the references to this item inside UIEngine are removed
	if (m_uiEngine)
		m_uiEngine->handleItemDestruction(this);
}

bool UIItem::handleMouseMove(const glm::vec2 mousePos)
{
	bool handled = false;
	if (isMouseHovering(mousePos))
	{
		handled = onMouseMove(mousePos);

		if (!getIsHovered())
		{
			onMouseEnter();

			setIsHovered(true);
		}
	}
	else
	{
		if (getIsHovered())
		{
			onMouseLeave();

			setIsHovered(false);
		}

		// mouse is not in any child so we don't bubble this event on childs
		handled = true;
	}

	return handled;
}

bool UIItem::handleMouseButtonPressed(int button, const glm::vec2& mousePos)
{
	bool handled = false;

	if (isMouseHovering(mousePos))
	{
		handled = onMouseButtonPressed(button, mousePos);

		m_uiEngine->selectItem(this);
		m_uiEngine->addPressedItem(this);
	}
	else
	{
		// mouse is not in any child so we don't bubble this event on childs
		handled = true;
	}

	return handled;
}

bool UIItem::handleMouseButtonReleased(int button, const glm::vec2& mousePos)
{
	bool handled = false;

	if (isMouseHovering(mousePos))
	{
		handled = onMouseButtonReleased(button, mousePos);
	}
	else
	{
		// mouse is not in any child so we don't bubble this event on childs
		handled = true;
	}

	return handled;
}

bool UIItem::handleKeyPressed(int key)
{
	bool handled = false;

	handled = onKeyPressed(key);

	return handled;
}

bool UIItem::handleKeyReleased(int key)
{
	bool handled = false;

	handled = onKeyReleased(key);

	return handled;
}

bool UIItem::handleCharacter(unsigned int codepoint)
{
	bool handled = false;

	handled = onCharacter(codepoint);

	return handled;
}

bool UIItem::handleDragOver(const glm::vec2& mousePos)
{
	bool handled = false;

	handled = onDragOver(mousePos);

	return handled;
}

bool UIItem::handleDrop(const glm::vec2& mousePos)
{
	bool handled = false;

	handled = onDrop(mousePos);

	return handled;
}