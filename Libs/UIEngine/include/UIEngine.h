#pragma once

#include <map>

#include "Widget.h"
#include "WidgetLayer.h"
#include "EmptyWidget.h"

class UIEngine
{
private:
	// Reources
	std::shared_ptr<VAO> m_rectShape;
	std::shared_ptr<ShaderProgram> m_UIWidgetProgram;
	std::shared_ptr<ShaderProgram> m_UIWidgetImageProgram;
	std::shared_ptr<ShaderProgram> m_UIWidgetTextProgram;
	// Factories
	std::map<std::string, std::function<std::shared_ptr<UIItem>()>> m_itemFactory;
	std::map<std::string, std::function<std::shared_ptr<Widget>()>> m_widgetFactory;
	std::map<std::string, std::function<std::shared_ptr<BaseWidgetLayer>()>> m_layerFactory;
	// Fonts
	FontFactory m_fontFactory;

	// Current displayed items
	//std::multimap<int, std::shared_ptr<BaseWidgetLayer>> m_layers;
	std::unique_ptr<ViewportWidget> m_rootViewportWidget;

	// Special item handling
	UIItem* m_selectedItem;
	std::vector<UIItem*> m_pressedItems;
	UIItem* m_draggedItem;

	glm::vec2 m_lastMousePressedPos;
	glm::vec2 m_mousePos;

public:
	UIEngine()
	{
		m_rootViewportWidget = std::make_unique<ViewportWidget>(this);

		// init resources
		m_rectShape = std::make_shared<VAO>();
		m_rectShape->setDatas(vertices, indices);
		m_UIWidgetProgram = std::make_shared<ShaderProgram>();
		m_UIWidgetProgram->load("resources/shaders/UIWidget.vert", "resources/shaders/UIWidget.frag");
		m_UIWidgetImageProgram = std::make_shared<ShaderProgram>();
		m_UIWidgetImageProgram->load("resources/shaders/UIImageWidget.vert", "resources/shaders/UIImageWidget.frag");
		m_UIWidgetTextProgram = std::make_shared<ShaderProgram>();
		m_UIWidgetTextProgram->load("resources/shaders/UITextWidget.vert", "resources/shaders/UITextWidget.frag");

		// init factories
		m_widgetFactory["EmptyWidget"] = [this]() { return std::make_shared<EmptyWidget>(this, this->getRectShape(), this->getUIWidgetProgram()); };
		m_widgetFactory["ImageWidget"] = [this]() { return std::make_shared<ImageWidget>(this, this->getRectShape(), this->getUIWidgetImageProgram()); };
		m_widgetFactory["TextWidget"] = [this]() { return std::make_shared<TextWidget>(this, this->getRectShape(), this->getUIWidgetTextProgram()); };
		m_widgetFactory["TextInputWidget"] = [this]() { return std::make_shared<TextInputWidget>(this, this->getRectShape(), this->getUIWidgetTextProgram(), this->getUIWidgetProgram()); };
		m_widgetFactory["ButtonWidget"] = [this]() { return std::make_shared<ButtonWidget>(this, this->getRectShape(), this->getUIWidgetProgram()); };
		m_widgetFactory["DropDownWidget"] = [this]() { return std::make_shared<DropDownWidget>(this, this->getRectShape(), this->getUIWidgetProgram()); };

		m_layerFactory["Raw"] = [this]() { return std::make_shared<RawLayer>(this); };
		m_layerFactory["Canvas"] = [this]() { return std::make_shared<CanvasLayer>(this); };
		m_layerFactory["HorizontalList"] = [this]() { return std::make_shared<HorizontalListLayer>(this); };
		m_layerFactory["VerticalList"] = [this]() { return std::make_shared<VerticalListLayer>(this); };
	}

	ViewportWidget* getRootViewportWidget()
	{
		return m_rootViewportWidget.get();
	}

	// factories instantiation
	std::shared_ptr<UIItem> instantiateUIItem(const std::string& itemTypeName)
	{
		auto found = m_itemFactory.find(itemTypeName);
		if (found != m_itemFactory.end())
		{
			return found->second();
		}
		else
		{
			return nullptr;
		}
	}
	std::shared_ptr<BaseWidgetLayer> instantiateLayer(const std::string& layerTypeName)
	{
		auto found = m_layerFactory.find(layerTypeName);
		if (found != m_layerFactory.end())
		{
			return found->second();
		}
		else
		{
			return nullptr;
		}
	}
	std::shared_ptr<Widget> instantiateWidget(const std::string& widgetTypeName)
	{
		auto found = m_widgetFactory.find(widgetTypeName);
		if (found != m_widgetFactory.end())
		{
			return found->second();
		}
		else
		{
			return nullptr;
		}
	}
	template<typename T>
	std::shared_ptr<T> instantiateWidgetAs(const std::string& widgetTypeName)
	{
		auto found = m_widgetFactory.find(widgetTypeName);
		if (found != m_widgetFactory.end())
		{
			return std::static_pointer_cast<T>(found->second());
		}
		else
		{
			return nullptr;
		}
	}

	// get resources
	std::shared_ptr<VAO> getRectShape() const
	{
		return m_rectShape;
	}
	std::shared_ptr<ShaderProgram> getUIWidgetProgram() const
	{
		return m_UIWidgetProgram;
	}
	std::shared_ptr<ShaderProgram> getUIWidgetImageProgram() const
	{
		return m_UIWidgetImageProgram;
	}
	std::shared_ptr<ShaderProgram> getUIWidgetTextProgram() const
	{
		return m_UIWidgetTextProgram;
	}
	FontFactory& getFontFactory()
	{
		return m_fontFactory;
	}
	
	// render all items
	void renderUI(const glm::vec2& viewportSize)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		ShaderProgram* currentBoundProgram = nullptr;

		m_rootViewportWidget->draw(&currentBoundProgram, viewportSize);

		currentBoundProgram = nullptr;

		glDisable(GL_BLEND);
	}

	// item handling
	//void addLayer(std::shared_ptr<BaseWidgetLayer> layer, int zorder)
	//{
	//	m_layers.insert(std::pair<int, std::shared_ptr<BaseWidgetLayer>>(zorder, layer));
	//}

	// input handling
	void handleMouseMove(const glm::vec2 mousePos)
	{
		m_mousePos = mousePos;

		// detect the drag
		if (!isDraggingItem())
		{
			for (auto& pressedItem : m_pressedItems)
			{
				if (pressedItem->isMouseHovering(m_lastMousePressedPos))
				{
					beginDragItem(pressedItem);
					break;
				}
			}
		}
		else
		{
			m_draggedItem->onDrag(m_mousePos);
			m_rootViewportWidget->handleDragOver(m_mousePos);
		}

		// handle mouse movements
		m_rootViewportWidget->handleMouseMove(mousePos);
	}
	void handleMouseButtonPressed(int button, const glm::vec2 mousePos)
	{
		m_lastMousePressedPos = mousePos;

		m_rootViewportWidget->handleMouseButtonPressed(button, mousePos);
	}
	void handleMouseButtonReleased(int button, const glm::vec2 mousePos)
	{
		// detect drag end
		if (isDraggingItem())
		{
			m_rootViewportWidget->handleDrop(mousePos);
			endDragItem();
		}

		// handle mouse button released
		m_rootViewportWidget->handleMouseButtonReleased(button, mousePos);
		// clear after we have handle the release event
		clearPressedItems();
	}

	void handleKeyPressed(int key)
	{
		m_rootViewportWidget->handleKeyPressed(key);
	}
	void handleKeyReleased(int key)
	{
		m_rootViewportWidget->handleKeyReleased(key);
	}
	void handleCharacter(unsigned int codepoint)
	{
		m_rootViewportWidget->handleCharacter(codepoint);
	}

	// selection
	void selectItem(UIItem* item)
	{
		deselectItem();

		m_selectedItem = item;
		
		if (m_selectedItem != nullptr)
			m_selectedItem->setIsSelected(true);
	}
	void deselectItem(UIItem* item)
	{
		if (m_selectedItem == item)
		{
			if (m_selectedItem != nullptr)
				m_selectedItem->setIsSelected(false);

			m_selectedItem = nullptr;
		}
	}
	void deselectItem()
	{
		if (m_selectedItem != nullptr)
			m_selectedItem->setIsSelected(false);

		m_selectedItem = nullptr;
	}
	UIItem* getSelectedItem() const
	{
		return m_selectedItem;
	}

	void clearPressedItems()
	{
		m_pressedItems.clear();
	}
	void addPressedItem(UIItem* item)
	{
		m_pressedItems.push_back(item);
	}

	bool isDraggingItem() const
	{
		return m_draggedItem != nullptr;
	}
	void beginDragItem(UIItem* item)
	{
		if (isDraggingItem())
			endDragItem();

		m_draggedItem = item;
		if (m_draggedItem != nullptr)
		{
			m_draggedItem->onDragBegin(m_mousePos);
		}
	}
	void endDragItem()
	{
		if (m_draggedItem != nullptr)
			m_draggedItem->onDragEnd(m_mousePos);
	}
	UIItem* getDraggedItem() const
	{
		return m_draggedItem;
	}

	void handleItemDestruction(UIItem* destroyedItem)
	{
		if (destroyedItem == m_selectedItem)
			deselectItem();
		auto& found = std::find(m_pressedItems.begin(), m_pressedItems.end(), destroyedItem);
		if (found != m_pressedItems.end())
		{
			m_pressedItems.erase(found);
		}
	}
};