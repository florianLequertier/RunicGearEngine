#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <map>
#include "Utils.h"
#include "UIItem.h"
#include "OpenglUtils.h"

#include "GLFW/glfw3.h" //Todo : remove dependency

class BaseWidgetLayer;
class WidgetSlot;

struct WidgetPadding
{
	float top;
	float bottom;
	float right;
	float left;

	WidgetPadding(float _top = 0, float _bottom = 0, float _right = 0, float _left = 0)
		: top(_top)
		, bottom(_bottom)
		, right(_right)
		, left(_left)
	{}

	bool operator==(const WidgetPadding& other) const
	{
		return top == other.top && bottom == other.bottom && right == other.right && left == other.left;
	}

	bool operator!=(const WidgetPadding& other) const
	{
		return !(operator==(other));
	}
};

struct WidgetAnchor
{
	bool hasProportionalScale;
	bool hasProportionalAnchorPosition;
	bool hasProportionalPositionRelativeToAnchor;

	glm::vec2 pivot; // clamped [0 -> 1]
	glm::vec2 anchorPosition; // clamped [0 -> 1] if hasProportionalAnchorPosition
	glm::vec2 positionRelativeToAnchor; // clamped [0 -> 1] if hasProportionalPositionRelativeToAnchor
	glm::vec2 selfSize; // clamped [0 -> 1] if hasProportionalScale

	WidgetAnchor()
		: hasProportionalScale(true)
		, hasProportionalAnchorPosition(true)
		, hasProportionalPositionRelativeToAnchor(true)
		, pivot(0, 0)
		, anchorPosition(0, 0)
		, positionRelativeToAnchor(0, 0)
		, selfSize(1, 1)
	{}
};

enum WidgetVisibility
{
	VISIBLE,
	INVISILE,
	COLLAPSED,
	HIT_TEST_INVISIBLE,
	SELF_HIT_TEST_INVISIBLE,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WidgetBase : public UIItem
{
protected:
	// the resulting bounding box in viewport
	Rect m_computedBounds;
	// the preferred size of this widget
	glm::vec2 m_preferredSize;
	// the computed position, relative to its parent layout
	glm::vec2 m_computedRelativePosition;

	WidgetVisibility m_visibility;

public:
	WidgetBase(UIEngine* uiengine);
	virtual ~WidgetBase()
	{}

	// visibility
	void setVisibility(WidgetVisibility visibility)
	{
		m_visibility = visibility;
	}
	WidgetVisibility getVisibility() const
	{
		return m_visibility;
	}

	// preferred size
	virtual void setPreferredSize(const glm::vec2& preferredSize)
	{
		m_preferredSize = preferredSize;
	}
	const glm::vec2& getPreferredSize() const
	{
		return m_preferredSize;
	}

	// computed transform
	// compute the position of the m_computedBounds based on parents position
	virtual void computePositionInViewport() = 0;
	virtual void computeChildrenPositionInViewport() = 0;
	virtual void computePositionInViewportRecur() = 0;

	void setComputedRelativePosition(const glm::vec2& position)
	{
		m_computedRelativePosition = position;
		m_computedBounds.pos = position;

		computePositionInViewport();
	}
	void setComputedSize(const glm::vec2& size)
	{
		m_computedBounds.extent = size;
	}
	const glm::vec2& getComputedRelativePosition() const
	{
		return m_computedRelativePosition;
	}
	const glm::vec2& getComputedSize() const
	{
		return m_computedBounds.extent;
	}
	const glm::vec2& getComputedPosition() const
	{
		return m_computedBounds.pos;
	}
	const Rect& getComputedBounds() const
	{
		return m_computedBounds;
	}
	void scaleComputedSizeBy(const glm::vec2& scaleFactor)
	{
		setComputedSize(getComputedSize() * scaleFactor);
	}
	void addOffsetToComputedPosition(const glm::vec2& offset)
	{
		setComputedRelativePosition(getComputedRelativePosition() + offset);
	}

	// hierarchy
	virtual WidgetSlot* getOwningSlot() const = 0;
	virtual BaseWidgetLayer* getOwningLayer() const = 0;
};

class Widget : public WidgetBase
{
	friend class WidgetSlot;

protected:
	std::string m_name;
	WidgetSlot* m_owningSlot;
	//std::vector<std::shared_ptr<Widget>> m_childs;

	std::shared_ptr<BaseWidgetLayer> m_layer;

	std::weak_ptr<VAO> m_shape;
	std::weak_ptr<ShaderProgram> m_program;

	//properties
	glm::vec4 m_tint;
	float m_cornerRadius;

public:
	Widget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> program);
	virtual ~Widget();

	// naming
	const std::string& getName() const;

	// layer and slot
	virtual WidgetSlot* getOwningSlot() const override;
	virtual BaseWidgetLayer* getOwningLayer() const override;
	void setLayer(const std::shared_ptr<BaseWidgetLayer>& layer);
	void removeLayer();
	BaseWidgetLayer* getLayer() const;
	virtual bool acceptLayer() const override;
	virtual void onWidgetAddedToLayer()
	{}

	//void addChild(std::shared_ptr<Widget> child);
	//void removeChild(Widget* child);

	// preferred size
	virtual void setPreferredSize(const glm::vec2& preferredSize)
	{
		WidgetBase::setPreferredSize(preferredSize);
		// first we assume that the computed size is the same as the preferred size
		setComputedSize(preferredSize);
		// then we recompute the size from parent to obtain the real computed size
		updateTransformFromParent();
	}
	//virtual void setSize(const glm::vec2& size, bool recursiveUpdateFromParent = false) override;
	//void setPosition(const glm::vec2& pos, bool recursiveUpdateFromParent = false);
	//void scaleBy(const glm::vec2& scaleFactor, bool recursiveUpdateFromParent = false);
	//void addOffset(const glm::vec2& offset, bool recursiveUpdateFromParent = false);

	virtual void computePositionInViewportRecur() override;
	virtual void computeChildrenPositionInViewport() override;
	virtual void computePositionInViewport() override;

	void updateTransformFromParent();
	void updateLayer(bool canUpdateParent = true, bool ignoreSizeToContent = false);

	// rendering
	virtual void draw(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const override;

	// events
	virtual bool isMouseHovering(const glm::vec2& cursor) const override;

	virtual bool handleMouseMove(const glm::vec2 mousePos) override;
	virtual bool handleMouseButtonPressed(int button, const glm::vec2& mousePos) override;
	virtual bool handleMouseButtonReleased(int button, const glm::vec2& mousePos) override;
	virtual bool handleKeyPressed(int key) override;
	virtual bool handleKeyReleased(int key) override;
	virtual bool handleCharacter(unsigned int codepoint) override;
	virtual bool handleDragOver(const glm::vec2& mousePos) override;
	virtual bool handleDrop(const glm::vec2& mousePos) override;

	// properties
	void setTint(const glm::vec4& tint)
	{
		m_tint = tint;
	}
	const glm::vec4& getTint() const
	{
		return m_tint;
	}
	void setCornerRadius(float cornerRadius)
	{
		m_cornerRadius = cornerRadius;
	}
	float getCornerRadius() const
	{
		return m_cornerRadius;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class WidgetSlot
{
protected:
	std::shared_ptr<Widget> m_ownedWidget;
	BaseWidgetLayer* m_owningLayer;

	bool m_sizeToContent;
	WidgetPadding m_padding;

public:
	WidgetSlot(BaseWidgetLayer* owningLayer, std::shared_ptr<Widget> ownedWidget);
	virtual ~WidgetSlot()
	{}

	Widget* getOwnedWidget() const;
	std::shared_ptr<Widget> getOwnedWidgetShared() const;
	BaseWidgetLayer* getOwningLayer() const;

	bool getSizeToContent() const;
	const WidgetPadding& getPadding() const;

	void setSizeToContent(bool sizeToContent);
	void setPadding(const WidgetPadding& padding);
};

class RawSlot : public WidgetSlot
{
private:
	glm::vec2 m_size;
	glm::vec2 m_position;

public:
	RawSlot(BaseWidgetLayer* _owningLayer, std::shared_ptr<Widget> _ownedWidget);
	virtual ~RawSlot();

	void setSize(const glm::vec2& size);
	const glm::vec2& getSize() const;

	void setPosition(const glm::vec2& position);
	const glm::vec2 getPosition() const;
};

class ListSlot : public WidgetSlot
{
private:
	bool m_fillX;
	bool m_fillY;

public:
	ListSlot(BaseWidgetLayer* _owningLayer, std::shared_ptr<Widget> _ownedWidget);
	virtual ~ListSlot();

	bool getFillX() const;
	bool getFillY() const;

	void setFillX(bool fillX);
	void setFillY(bool fillY);
};

class CanvasSlot final : public WidgetSlot
{
private:
	WidgetAnchor m_anchor;
	glm::vec2 m_size;

public:
	CanvasSlot(BaseWidgetLayer* _owningLayer, std::shared_ptr<Widget> _ownedWidget);
	virtual ~CanvasSlot();

	void setSize(const glm::vec2& size);
	const glm::vec2& getSize() const;

	WidgetAnchor& getAnchor();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ViewportWidget final : public WidgetBase
{
private:
	// Current displayed items
	std::multimap<int, std::shared_ptr<BaseWidgetLayer>> m_layers;

public:
	ViewportWidget(UIEngine* uiengine)
		: WidgetBase(uiengine)
	{}

	virtual void draw(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const override;
	virtual bool isMouseHovering(const glm::vec2& cursor) const override;
	virtual bool acceptLayer() const override;
	// has no owning slot
	virtual WidgetSlot* getOwningSlot() const override { return nullptr; }
	// has no owning layer
	virtual BaseWidgetLayer* getOwningLayer() const override { return nullptr; }

	void setViewport(const glm::vec2& pos, const glm::vec2& extent);
	//void setSize(const glm::vec2& size, bool updateChilds = true) override;
	void computePositionInViewportRecur() override;
	virtual void computeChildrenPositionInViewport() override;
	void computePositionInViewport() override;

	// layer handling
	void addLayer(std::shared_ptr<BaseWidgetLayer> layer, int zorder);
	void removeLayer(std::shared_ptr<BaseWidgetLayer> layer);

	// events
	virtual bool handleMouseMove(const glm::vec2 mousePos) override;
	virtual bool handleMouseButtonPressed(int button, const glm::vec2& mousePos) override;
	virtual bool handleMouseButtonReleased(int button, const glm::vec2& mousePos) override;
	virtual bool handleKeyPressed(int key) override;
	virtual bool handleKeyReleased(int key) override;
	virtual bool handleCharacter(unsigned int codepoint) override;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ImageWidget : public Widget
{
private:
	std::shared_ptr<Texture> m_texture;

public:
	ImageWidget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> program);
	virtual ~ImageWidget();

	void setTexture(std::shared_ptr<Texture> texture);
	const Texture* getTexture() const;

	virtual void draw(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const override;
};

class TextWidget : public Widget
{
private:
	std::shared_ptr<Font> m_font;
	std::string m_text;
	Rect m_textBounds;

public:
	TextWidget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> program);
	virtual ~TextWidget();

	void setFont(std::shared_ptr<Font> font);
	const Font* getFont() const;
	void setText(const std::string& text);
	const std::string& getText() const;

	virtual void draw(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const override;
	void drawText(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const;
};

class TextInputWidget : public Widget
{
public:
	std::function<void()> onTextCommit;

private:
	std::shared_ptr<Font> m_font;
	std::string m_text;
	Rect m_textBounds;
	int m_cursorPos;
	bool m_isEditing;
	std::weak_ptr<ShaderProgram> m_cursorProgram;

public:
	TextInputWidget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> textProgram, std::weak_ptr<ShaderProgram> cursorProgram);
	virtual ~TextInputWidget();

	void setFont(std::shared_ptr<Font> font);
	const Font* getFont() const;
	void setText(const std::string& text);
	const std::string& getText() const;
	void addCharacter(char character);
	void removePreviousCharacter();
	void removeNextCharacter();
	void cursorNext();
	void cursorPrevious();

	virtual void draw(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const override;
	void drawText(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const;
	void drawCursor(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const;

	virtual bool onKeyPressed(int key) override
	{
		if (getIsSelected())
		{
			if(key == GLFW_KEY_BACKSPACE)
				removePreviousCharacter();
			if (key == GLFW_KEY_DELETE)
				removeNextCharacter();
			else if (key == GLFW_KEY_RIGHT)
			{
				cursorNext();
			}
			else if (key == GLFW_KEY_LEFT)
			{
				cursorPrevious();
			}

			return true;
		}
		return false;
	}
	virtual bool onCharacter(unsigned int codepoint) override
	{ 
		if (getIsSelected())
		{
			addCharacter(codepoint);

			return true; 
		}
		return false;
	}
	virtual bool onMouseButtonPressed(int button, const glm::vec2& mousePos) override
	{
		glm::vec2 relativePos = mousePos - m_computedBounds.pos;
		m_cursorPos = m_font->getCursorIdx(m_text, relativePos);

		return true;
	}
};

struct ButtonStyle
{
	glm::vec4 defaultTint;
	WidgetPadding defaultPadding;

	glm::vec4 hoveredTint;
	WidgetPadding hoveredPadding;

	glm::vec4 pressedTint;
	WidgetPadding pressedPadding;

	ButtonStyle()
		: defaultTint(glm::vec4(0.5, 0.5, 0.5, 1.0))
		, defaultPadding(WidgetPadding(0, 0, 0, 0))
		, hoveredTint(glm::vec4(1.0, 0.7, 0.7, 1.0))
		, hoveredPadding(WidgetPadding(0, 0, 0, 0))
		, pressedTint(glm::vec4(0.3, 0.3, 0.3, 1.0))
		, pressedPadding(WidgetPadding(0, 0, 0, 0))
	{}

	ButtonStyle(const glm::vec4& _defaultTint, const WidgetPadding& _defaultPadding
		, const glm::vec4& _hoveredTint, const WidgetPadding& _hoveredPadding
		, const glm::vec4& _pressedTint,const WidgetPadding _pressedPadding)
		: defaultTint(_defaultTint)
		, defaultPadding(_defaultPadding)
		, hoveredTint(_hoveredTint)
		, hoveredPadding(_hoveredPadding)
		, pressedTint(_pressedTint)
		, pressedPadding(_pressedPadding)
	{}
};

enum ButtonState {
	DEFAULT,
	HOVERRED,
	PRESSED,
};

class ButtonWidget : public Widget
{
public:
	// mouse button, mouse position
	std::function<void(int, const glm::vec2&)> onClicked;

private:
	ButtonStyle m_style;
	bool m_waitReleased;
	ButtonState m_buttonState;

public:
	ButtonWidget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> textProgram)
		: Widget(uiengine, shape, textProgram)
		, m_buttonState(ButtonState::DEFAULT)
		, m_waitReleased(false)
	{
	}
	virtual ~ButtonWidget()
	{}

	virtual void onWidgetAddedToLayer() override
	{
		updateStyle();
	}
	void setButtonState(ButtonState state)
	{
		m_buttonState = state;
		updateStyle();
	}
	void updateStyle()
	{
		switch (m_buttonState)
		{
		case DEFAULT:
			if (m_owningSlot != nullptr)
				getOwningSlot()->setPadding(m_style.defaultPadding);
			setTint(m_style.defaultTint);
			break;
		case HOVERRED:
			if (m_owningSlot != nullptr)
				getOwningSlot()->setPadding(m_style.hoveredPadding);
			setTint(m_style.hoveredTint);
			break;
		case PRESSED:
			if (m_owningSlot != nullptr)
				getOwningSlot()->setPadding(m_style.pressedPadding);
			setTint(m_style.pressedTint);
			break;
		default:
			break;
		}
	}

	// style
	void setStyle(const ButtonStyle& style)
	{
		m_style = style;
		updateStyle();
	}
	ButtonStyle& getStyleForWrite()
	{
		return m_style;
	}
	const ButtonStyle& getStyle() const
	{
		return m_style;
	}

	// event
	virtual bool onMouseEnter() override
	{
		setButtonState(ButtonState::HOVERRED);

		return true;
	}
	virtual bool onMouseLeave() override
	{
		setButtonState(ButtonState::DEFAULT);

		m_waitReleased = false;

		return true;
	}
	virtual bool onMouseButtonPressed(int button, const glm::vec2& mousePos) override
	{
		setButtonState(ButtonState::PRESSED);

		m_waitReleased = true;

		return true;
	}
	virtual bool onMouseButtonReleased(int button, const glm::vec2& mousePos) override
	{
		setButtonState(ButtonState::HOVERRED);

		if (m_pressed && m_waitReleased)
		{
			if (onClicked)
				onClicked(button, mousePos);

			m_waitReleased = false;
		}

		return true;
	}
};

class DropDownWidget : public Widget
{
private:
	std::shared_ptr<BaseWidgetLayer> m_dropDownListLayout;
	Widget* m_dropDownList;
	BaseWidgetLayer* m_dropDownListContent;

	std::vector<std::string> m_labels;
	ButtonWidget* m_selection;
	TextWidget* m_selectionText;
	int m_currentSelectedItem;
	std::string m_emptyItemListText;

public:
	DropDownWidget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> shaderProgram);
	virtual ~DropDownWidget();

	// drop down list
	void displayDropDownList();
	void hideItemDropDown();
	void toggleDropDownListVisibility();
	virtual void onItemUnselected() override;

	// option items
	std::shared_ptr<Widget> createDefaultItem(const std::string& label, int indexInList);
	void addDefaultItem(const std::string& label);
	void clearItems();
	void setDefaultItems(const std::vector<std::string>& labels);
	void selectItem(int itemIdx);
	const std::string& getSelectedItemLabel() const;
	int getSelectedItemIdx() const;
};

class WindowWidget : public Widget
{
private:
	Widget* m_titleBar;
	TextWidget* m_titleText;
	Widget* m_menu;
	BaseWidgetLayer* m_menuLayout;
	glm::vec2 m_dragOffset;
	WindowFrameSlot* m_content;

public:
	WindowWidget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> shaderProgram);
	virtual ~WindowWidget()
	{}

	// title
	void setTitleText(const std::string& title);
	const std::string& getTitleText() const;
	void setDisplayTitleBar(bool dislayTitleBar);

	// menu
	void setDisplayMenu(bool displayMenu);
	void addMenuItem(std::shared_ptr<Widget> item);
	void removeMenuItem(const Widget* item);

	void getAllFramesRecurShared(std::vector<std::shared_ptr<WindowFrame>>& outFrames);
};

class WindowFrameSlotContent : public Widget
{
protected:
	WindowFrameSlot* m_windowFrameSlot;

public:
	WindowFrameSlotContent(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> shaderProgram);
	virtual ~WindowFrameSlotContent();

	void setWindowSlot(WindowFrameSlot* windowSlot);
};

class WindowFrameSlot : public WindowFrameSlotContent
{
private:
	bool m_isHorizontalLayer;
	Widget* m_dropAnchor;
	Widget* m_content;

public:

	WindowFrameSlot(bool isHorizontal, UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> shaderProgram);
	virtual ~WindowFrameSlot();

	bool isSlotEmpty() const;

	int getWidgetChildCount() const;
	WindowFrameSlotContent* getWidgetChildAsFrameSlotContent(int childIndex);
	std::shared_ptr<WindowFrameSlotContent> getWidgetChildSharedAsFrameSlotContent(int childIndex);

	void handleWindowDrop();
	void addWindowFrames(WindowWidget* window);
	void addFirstFrame(const std::shared_ptr<WindowFrame>& frameToAdd);

	void splitAndAdd(int indexOffset, bool requetedAlignmentIsHorizontal, const WindowFrame* childFrame, const std::shared_ptr<WindowFrameSlot>& frameSlotToAdd);

	void splitAddRight(const WindowFrame* childFrame, const std::shared_ptr<WindowFrameSlot>& frameSlotToAdd);
	void splitAddLeft(const WindowFrame* childFrame, const std::shared_ptr<WindowFrameSlot>& frameSlotToAdd);
	void splitAddTop(const WindowFrame* childFrame, const std::shared_ptr<WindowFrameSlot>& frameSlotToAdd);
	void splitAddBottom(const WindowFrame* childFrame, const std::shared_ptr<WindowFrameSlot>& frameSlotToAdd);

	void displayDropAnchor();
	void HideDropAnchor();

	void getAllFramesRecurShared(std::vector<std::shared_ptr<WindowFrame>>& outFrames);
};

class WindowFrame : public WindowFrameSlotContent
{
private:
	std::shared_ptr<Widget> m_mainContainer;
	Widget* m_tabsContainer;
	Widget* m_frameContainer;
	std::shared_ptr<Widget> m_frameContent;
	Widget* m_dropAnchor;

	WindowFrameSlot* m_WindowSlot;

	int m_selectedTab;

public:
	WindowFrame(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> shaderProgram);
	virtual ~WindowFrame();

	void addContent(const std::shared_ptr<Widget>& content);

	void displayContent(int tabIndex);

	void addWindowFrames(WindowWidget* window);
	void handleWindowDropMiddle();
	void handleWindowDropRight(const std::shared_ptr<WindowFrameSlot>& frameSlot);
	void handleWindowDropLeft(const std::shared_ptr<WindowFrameSlot>& frameSlot);
	void handleWindowDropTop(const std::shared_ptr<WindowFrameSlot>& frameSlot);
	void handleWindowDropBottom(const std::shared_ptr<WindowFrameSlot>& frameSlot);

	void setWindowSlot(WindowFrameSlot* slot);

	void displayDropAnchor();
	void HideDropAnchor();

	std::shared_ptr<Widget> getContentShared() const;
};