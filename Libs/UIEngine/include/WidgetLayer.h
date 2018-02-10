#pragma once

#include "UIItem.h"
#include "Widget.h"

class BaseWidgetLayer// : public UIItem
{
protected:
	UIEngine* m_uiengine;
	WidgetBase* m_owningWidget;

public:
	BaseWidgetLayer(UIEngine* uiengine)
		: m_uiengine(uiengine)
		, m_owningWidget(nullptr)
	{}
	virtual ~BaseWidgetLayer()
	{}

	// owning widget handling
	void setOwningWidget(WidgetBase* owningWidget);
	bool isAttachedToWidget() const;
	WidgetBase* getOwningWidget() const
	{
		return m_owningWidget;
	}
	WidgetSlot* getOwningWidgetSlot() const
	{
		if (isAttachedToWidget())
			return m_owningWidget->getOwningSlot();
		else
			return nullptr;
	}

	// transform
	const glm::vec2& getPosition() const
	{
		// We take the padding into account to place the layer into its owning widget
		auto owningSlot = getOwningWidgetSlot();
		float paddingOffsetTop = owningSlot == nullptr ? 0 : owningSlot->getPadding().top;
		float paddingOffsetLeft = owningSlot == nullptr ? 0 : owningSlot->getPadding().left;

		if (m_owningWidget != nullptr)
			return m_owningWidget->getComputedPosition() + glm::vec2(paddingOffsetLeft, paddingOffsetTop);
		else
			return glm::vec2(0, 0);
	}

	const glm::vec2& getRelativePosition() const
	{
		// We take the padding into account to place the layer into its owning widget
		auto owningSlot = getOwningWidgetSlot();
		float paddingOffsetTop = owningSlot == nullptr ? 0 : owningSlot->getPadding().top;
		float paddingOffsetLeft = owningSlot == nullptr ? 0 : owningSlot->getPadding().left;

		if (m_owningWidget != nullptr)
			return m_owningWidget->getComputedRelativePosition() + +glm::vec2(paddingOffsetLeft, paddingOffsetTop);
		else
			return glm::vec2(0, 0);
	}

	const glm::vec2& getSize() const
	{
		// We take the padding into account to place the layer into its owning widget
		auto owningSlot = getOwningWidgetSlot();
		float paddingDeltaSizeW = owningSlot == nullptr ? 0 : owningSlot->getPadding().left + owningSlot->getPadding().right;
		float paddingDeltaSizeH = owningSlot == nullptr ? 0 : owningSlot->getPadding().top + owningSlot->getPadding().bottom;

		if (m_owningWidget != nullptr)
			return m_owningWidget->getComputedSize() - glm::vec2(paddingDeltaSizeW, paddingDeltaSizeH);
		else
			return glm::vec2(0, 0);
	}

	virtual void computePositionInViewportRecur() = 0;
	virtual void computeChildrenPositionInViewport() = 0;

	// slots handling
	virtual WidgetSlot* addSlot(std::shared_ptr<Widget> widget) = 0;
	template<typename SlotType>
	SlotType* addSlotAs(std::shared_ptr<Widget> widget)
	{
		return static_cast<SlotType*>(addSlot(widget));
	}
	virtual void removeSlot(int slotIndex) = 0;
	virtual void removeSlot(const UIItem* ownedItem) = 0;
	virtual void clearSlots() = 0;
	virtual void updateSlotsRecur(bool canUpdateParent = true, bool ignoreSizeToContent = false) = 0;
	virtual void updateSlotsRects(bool canUpdateParent = true, bool ignoreSizeToContent = false) = 0;
	virtual std::shared_ptr<WidgetSlot> getSlotShared(int slotIndex) const = 0;
	virtual WidgetSlot* getSlot(int slotIndex) const = 0;
	virtual int getSlotCount() const = 0;
	virtual WidgetSlot* insertSlot(std::shared_ptr<Widget> widget, int index) = 0;
	template<typename SlotType>
	virtual SlotType* insertSlotAs(std::shared_ptr<Widget> widget, int index)
	{
		return static_cast<SlotType>(insertSlot(widget, index));
	}

	// rendering
	virtual void draw(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const = 0;

	// inputs
	virtual bool isMouseHovering(const glm::vec2& cursor) const = 0;
	virtual bool handleMouseMove(const glm::vec2 mousePos) = 0;
	virtual bool handleMouseButtonPressed(int button, const glm::vec2& mousePos) = 0;
	virtual bool handleMouseButtonReleased(int button, const glm::vec2& mousePos) = 0;
	virtual bool handleKeyPressed(int key) = 0;
	virtual bool handleKeyReleased(int key) = 0;
	virtual bool handleCharacter(unsigned int codepoint) = 0;
	virtual bool handleDragOver(const glm::vec2& mousePos) = 0;
	virtual bool handleDrop(const glm::vec2& mousePos) = 0;
	virtual bool handleDrag(const glm::vec2& mousePos) = 0;
};

template<typename SlotClass>
class WidgetLayer : public BaseWidgetLayer
{
protected:
	std::vector<std::shared_ptr<SlotClass>> m_slots;

public:
	WidgetLayer(UIEngine* uiengine)
		: BaseWidgetLayer(uiengine)
	{}
	virtual ~WidgetLayer()
	{}

	WidgetSlot* addSlot(std::shared_ptr<Widget> widget) override
	{
		// The child widget MUST be different than the owning widget !
		assert(widget.get() != m_owningWidget);

		auto newSlot = std::make_shared<SlotClass>(this, widget);
		m_slots.push_back(newSlot);
		updateSlotsRecur();
		widget->onWidgetAddedToLayer();

		return newSlot.get();
	}
	void removeSlot(int slotIndex) override
	{
		m_slots.erase(m_slots.begin() + slotIndex);
		updateSlotsRecur();
	}
	void removeSlot(const UIItem* ownedItem) override
	{
		auto& found = std::find_if(m_slots.begin(), m_slots.end(), [ownedItem](const std::shared_ptr<SlotClass>& item) { return item->getOwnedWidget() == ownedItem; });
		if (found != m_slots.end())
		{
			m_slots.erase(found);
		}
		updateSlotsRecur();
	}
	void clearSlots() override
	{
		m_slots.clear();
		updateSlotsRecur();
	}
	std::shared_ptr<WidgetSlot> getSlotShared(int slotIndex) const override
	{
		if (slotIndex < 0 || slotIndex >= m_slots.size())
			return nullptr;

		return m_slots[slotIndex];
	}
	WidgetSlot* getSlot(int slotIndex) const override
	{
		if (slotIndex < 0 || slotIndex >= m_slots.size())
			return nullptr;

		return m_slots[slotIndex].get();
	}
	int getSlotCount() const override
	{
		return m_slots.size();
	}
	WidgetSlot* insertSlot(std::shared_ptr<Widget> widget, int index) override
	{
		auto newSlot = std::make_shared<SlotClass>(this, widget);
		m_slots.insert(m_slots.begin() + index, newSlot);

		updateSlotsRecur();
		widget->onWidgetAddedToLayer();

		return newSlot;
	}
	
	void updateSlotsRecur(bool canUpdateParent = true, bool ignoreSizeToContent = false) override
	{
		// first update child recursivly
		for (auto& slot : m_slots)
		{
			slot->getOwnedWidget()->updateLayer(false, ignoreSizeToContent);
		}

		// then update self
		if(m_owningWidget != nullptr && getOwningWidgetSlot() != nullptr)
			updateSlotsRects(canUpdateParent, ignoreSizeToContent);

		// make sur all position relative to viewport are up to date
		for (auto& slot : m_slots)
		{
			slot->getOwnedWidget()->computeChildrenPositionInViewport();
		}
		//computePositionInViewportRecur();
	}

	virtual void computePositionInViewportRecur() override
	{
		for (auto& slot : m_slots)
		{
			slot->getOwnedWidget()->computePositionInViewportRecur();
		}
	}

	virtual void computeChildrenPositionInViewport() override
	{
		for (auto& slot : m_slots)
		{
			slot->getOwnedWidget()->computePositionInViewport();
		}
	}

	//virtual void computePositionInViewport() override
	//{
	//	// nothing to do here
	//}

	virtual void draw(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const override
	{
		for (const auto& slot : m_slots)
		{
			slot->getOwnedWidget()->draw(boundProgram, viewportSize);
		}
	}
	virtual bool isMouseHovering(const glm::vec2& cursor) const override
	{
		return m_owningWidget->isMouseHovering(cursor);
	}

	virtual bool handleMouseMove(const glm::vec2 mousePos) override
	{
		bool handled = false;
		for (auto& slot : m_slots)
		{
			bool childHandled = slot->getOwnedWidget()->handleMouseMove(mousePos);
			handled = handled || childHandled;
		}

		return handled;
	}
	virtual bool handleMouseButtonPressed(int button, const glm::vec2& mousePos) override
	{
		bool handled = false;
		for (auto& slot : m_slots)
		{
			bool childHandled = slot->getOwnedWidget()->handleMouseButtonPressed(button, mousePos);
			handled = handled || childHandled;
		}

		return handled;
	}
	virtual bool handleMouseButtonReleased(int button, const glm::vec2& mousePos) override
	{
		bool handled = false;
		for(auto& slot : m_slots)
		{
			bool childHandled = slot->getOwnedWidget()->handleMouseButtonReleased(button, mousePos);
			handled = handled || childHandled;
		}

		return handled;
	}
	virtual bool handleKeyPressed(int key) override
	{
		bool handled = false;
		for (auto& slot : m_slots)
		{
			bool childHandled = slot->getOwnedWidget()->handleKeyPressed(key);
			handled = handled || childHandled;
		}

		return handled;
	}
	virtual bool handleKeyReleased(int key) override
	{
		bool handled = false;
		for (auto& slot : m_slots)
		{
			bool childHandled = slot->getOwnedWidget()->handleKeyReleased(key);
			handled = handled || childHandled;
		}

		return handled;
	}
	virtual bool handleCharacter(unsigned int codepoint) override
	{
		bool handled = false;
		for (auto& slot : m_slots)
		{
			bool childHandled = slot->getOwnedWidget()->handleCharacter(codepoint);
			handled = handled || childHandled;
		}

		return handled;
	}
	virtual bool handleDragOver(const glm::vec2& mousePos) override
	{
		bool handled = false;
		for (auto& slot : m_slots)
		{
			bool childHandled = slot->getOwnedWidget()->handleDragOver(mousePos);
			handled = handled || childHandled;
		}

		return handled;
	}
	virtual bool handleDrop(const glm::vec2& mousePos) override
	{
		bool handled = false;
		for (auto& slot : m_slots)
		{
			bool childHandled = slot->getOwnedWidget()->handleDrop(mousePos);
			handled = handled || childHandled;
		}

		return handled;
	}
	virtual bool handleDrag(const glm::vec2& mousePos) override
	{
		bool handled = false;
		for (auto& slot : m_slots)
		{
			bool childHandled = slot->getOwnedWidget()->handleDrag(mousePos);
			handled = handled || childHandled;
		}

		return handled;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RawLayer final : public WidgetLayer<RawSlot>
{
public:
	RawLayer(UIEngine* uiengine)
		: WidgetLayer<RawSlot>(uiengine)
	{}
	virtual ~RawLayer()
	{}

	void updateSlotsRects(bool canUpdateParent, bool ignoreSizeToContent = false) override
	{
		for (auto& slot : m_slots)
		{
			glm::vec2 slotInitialSize = slot->getOwnedWidget()->getVisibility() == WidgetVisibility::COLLAPSED ? glm::vec2(0, 0) : slot->getOwnedWidget()->getPreferredSize();
			slot->getOwnedWidget()->setComputedSize(slotInitialSize);
			slot->getOwnedWidget()->setComputedRelativePosition(slot->getPosition());
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CanvasLayer final : public WidgetLayer<CanvasSlot>
{
public:
	CanvasLayer(UIEngine* uiengine)
		: WidgetLayer<CanvasSlot>(uiengine)
	{}
	virtual ~CanvasLayer()
	{}

	void updateSlotsRects(bool canUpdateParent, bool ignoreSizeToContent = false) override
	{
		// parent size (with padding)
		glm::vec2 parentSize = getSize();
		Rect contentBounds;
		int slotIndex = 0;
		for (auto& slot : m_slots)
		{
			// reset the size to the preferred size before computation
			glm::vec2 slotInitialSize = slot->getOwnedWidget()->getVisibility() == WidgetVisibility::COLLAPSED ? glm::vec2(0, 0) : slot->getOwnedWidget()->getPreferredSize();
			slot->getOwnedWidget()->setComputedSize(slotInitialSize);

			// self size
			glm::vec2 selfSlotSize = slot->getAnchor().selfSize;
			if (slot->getSizeToContent())
			{
				slot->getOwnedWidget()->updateLayer(false);

				selfSlotSize = slot->getOwnedWidget()->getComputedSize();
			}
			else if (slot->getAnchor().hasProportionalScale)
			{
				selfSlotSize *= parentSize;
			}
			slot->getOwnedWidget()->setComputedSize(selfSlotSize);

			// anchor to container position
			glm::vec2 anchorPosition = slot->getAnchor().anchorPosition;
			if (slot->getAnchor().hasProportionalAnchorPosition)
			{
				anchorPosition *= parentSize;
			}

			// self anchor position
			glm::vec2 positionRelativeToAnchor = slot->getAnchor().positionRelativeToAnchor;
			if (slot->getAnchor().hasProportionalPositionRelativeToAnchor)
			{
				positionRelativeToAnchor *= parentSize;
			}

			// pivot
			glm::vec2 pivotValue = -slot->getAnchor().pivot * selfSlotSize;
			slot->getOwnedWidget()->setComputedRelativePosition(anchorPosition + positionRelativeToAnchor);
			slot->getOwnedWidget()->addOffsetToComputedPosition(pivotValue);

			// update childs
			if(!slot->getSizeToContent())
				slot->getOwnedWidget()->updateLayer(false);

			// compute content bounds
			if (slotIndex == 0)
				contentBounds = slot->getOwnedWidget()->getComputedBounds();
			else
				contentBounds.append(slot->getOwnedWidget()->getComputedBounds());

			slotIndex++;
		}

		// if we are inside a widget which is setup to resize to its content, then we modify its size if the content size has changed, and we update its parent
		if (!ignoreSizeToContent && getOwningWidgetSlot() != nullptr && getOwningWidgetSlot()->getOwningLayer() != nullptr
			&& getOwningWidgetSlot()->getSizeToContent() && contentBounds.extent != getOwningWidget()->getComputedSize())
		{
			m_owningWidget->setComputedSize(contentBounds.extent);

			if(canUpdateParent)
				m_owningWidget->getOwningLayer()->updateSlotsRects(false);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class HorizontalListLayer final : public WidgetLayer<ListSlot>
{
public:
	HorizontalListLayer(UIEngine* uiengine)
		: WidgetLayer<ListSlot>(uiengine)
	{}

	void updateSlotsRects(bool canUpdateParent, bool ignoreSizeToContent = false) override
	{
		// We can't compute rect if we are not attached to a slot
		if (getOwningWidgetSlot() == nullptr)
			return;

		// We resize the childs only if the parent widget is not set as a sizeToContent widget
		if (!getOwningWidgetSlot()->getSizeToContent())
		{
			// compute the availlable space for slots which fill on X and for slot which keep their preferred size
			glm::vec2 availlableSize = getSize();
			float availlableSizeForFilledX = availlableSize.x;
			float availlableSizeForFilledY = availlableSize.y;
			int slotPreferedSizeXCount = 0;
			int slotFilledSizeXCount = 0;

			for (auto& slot : m_slots)
			{
				// reset the size to the preferred size before computation
				glm::vec2 slotInitialSize = slot->getOwnedWidget()->getVisibility() == WidgetVisibility::COLLAPSED ? glm::vec2(0, 0) : slot->getOwnedWidget()->getPreferredSize();
				slot->getOwnedWidget()->setComputedSize(slotInitialSize);

				bool fillX = slot->getFillX();
				bool isSizedToContent = slot->getSizeToContent();
				if (!fillX || isSizedToContent)
				{
					// update child to get its computed size
					slot->getOwnedWidget()->updateLayer(false);

					availlableSizeForFilledX -= slot->getOwnedWidget()->getComputedSize().x;
					slotPreferedSizeXCount++;
				}
				else
				{
					slotFilledSizeXCount++;
				}
			}

			// resize the slots which doesn't fill on X if there is not enought space for them
			if (availlableSizeForFilledX < 0 && slotPreferedSizeXCount > 0)
			{
				float forceScale = availlableSize.x / (availlableSize.x - availlableSizeForFilledX);
				for (auto& slot : m_slots)
				{
					if (!slot->getFillX())
					{
						slot->getOwnedWidget()->scaleComputedSizeBy(glm::vec2(forceScale, 1.0f));
						// We update the child computed bounds
						slot->getOwnedWidget()->updateLayer(false, true);
					}
				}

				availlableSizeForFilledX = 0;
			}

			// resize the fillX slots
			if (slotFilledSizeXCount > 0)
			{
				float availlableSizeForOneFilled = availlableSizeForFilledX / slotFilledSizeXCount;
				for (auto& slot : m_slots)
				{
					if (slot->getFillX() && slot->getFillY())
					{
						slot->getOwnedWidget()->setComputedSize(glm::vec2(availlableSizeForOneFilled, availlableSizeForFilledY));
						// We update the child computed bounds
						slot->getOwnedWidget()->updateLayer(false);
					}
					else if (slot->getFillX() && !slot->getFillY())
					{
						glm::vec2 slotSize = slot->getOwnedWidget()->getComputedSize();
						slot->getOwnedWidget()->setComputedSize(glm::vec2(availlableSizeForOneFilled, slotSize.y));
						// We update the child computed bounds
						slot->getOwnedWidget()->updateLayer(false);
					}
					else if (!slot->getFillX() && slot->getFillY())
					{
						glm::vec2 slotSize = slot->getOwnedWidget()->getComputedSize();
						slot->getOwnedWidget()->setComputedSize(glm::vec2(slotSize.x, availlableSizeForFilledY));
						// We update the child computed bounds
						slot->getOwnedWidget()->updateLayer(false);
					}
				}
			}

			// Once we have modified the size of childs, we need to update them
			//for (auto& slot : m_slots)
			//{
			//	if(!slot->getSizeToContent())
			//		slot->getOwnedWidget()->updateLayer();
			//}
		}

		// update slots position
		glm::vec2 cursor(0, 0);
		float contentSizeX = 0;
		float contentSizeY = 0;
		for (auto& slot : m_slots)
		{
			slot->getOwnedWidget()->setComputedRelativePosition(cursor);
			cursor.x += slot->getOwnedWidget()->getComputedSize().x;
			contentSizeY = std::max(contentSizeY, slot->getOwnedWidget()->getComputedSize().y);
		}
		contentSizeX = cursor.x;

		// If the current widget is a sizeToContent widget, we update its size based on the content
		if (!ignoreSizeToContent && getOwningWidgetSlot()->getSizeToContent())
		{
			// We update only if the size has changed to avoid circulary calls
			const glm::vec2& computedSize = m_owningWidget->getComputedSize();
			if (computedSize != glm::vec2(contentSizeX, contentSizeY))
			{
				m_owningWidget->setComputedSize(glm::vec2(contentSizeX, contentSizeY));

				if (canUpdateParent)
				{
					// this time we update the parent
					m_owningWidget->getOwningSlot()->getOwningLayer()->updateSlotsRects(false);
				}
			}
		}
	}
};


class VerticalListLayer final : public WidgetLayer<ListSlot>
{
public:
	VerticalListLayer(UIEngine* uiengine)
		: WidgetLayer<ListSlot>(uiengine)
	{}

	void updateSlotsRects(bool canUpdateParent, bool ignoreSizeToContent = false) override
	{
		// We can't compute rect if we are not attached to a slot
		if (getOwningWidgetSlot() == nullptr)
			return;

		// We resize the childs only if the parent widget is not set as a sizeToContent widget
		if (!getOwningWidgetSlot()->getSizeToContent())
		{
			// compute the availlable space for slots which fill on X and for slot which keep their preferred size
			glm::vec2 availlableSize = getSize();
			float availlableSizeForFilledX = availlableSize.x;
			float availlableSizeForFilledY = availlableSize.y;
			int slotPreferedSizeYCount = 0;
			int slotFilledSizeYCount = 0;

			for (auto& slot : m_slots)
			{
				// reset the size to the preferred size before computation
				glm::vec2 slotInitialSize = slot->getOwnedWidget()->getVisibility() == WidgetVisibility::COLLAPSED ? glm::vec2(0, 0) : slot->getOwnedWidget()->getPreferredSize();
				slot->getOwnedWidget()->setComputedSize(slotInitialSize);

				bool fillY = slot->getFillY();
				bool isSizedToContent = slot->getSizeToContent();
				if (!fillY || isSizedToContent)
				{
					// update child to get its computed size
					slot->getOwnedWidget()->updateLayer(false);

					availlableSizeForFilledY -= slot->getOwnedWidget()->getComputedSize().y;
					slotPreferedSizeYCount++;
				}
				else
				{
					slotFilledSizeYCount++;
				}
			}

			// resize the slots which doesn't fill on y if there is not enought space for them
			if (availlableSizeForFilledY < 0 && slotPreferedSizeYCount > 0)
			{
				float forceScale = availlableSize.y / (availlableSize.y - availlableSizeForFilledY);
				for (auto& slot : m_slots)
				{
					if (!slot->getFillY())
					{
						slot->getOwnedWidget()->scaleComputedSizeBy(glm::vec2(forceScale, 1.0f));
						// We update the child computed bounds
						slot->getOwnedWidget()->updateLayer(false, true);
					}
				}

				availlableSizeForFilledY = 0;
			}

			// resize the fillY slots
			if (slotFilledSizeYCount > 0)
			{
				float availlableSizeForOneFilled = availlableSizeForFilledY / slotFilledSizeYCount;
				for (auto& slot : m_slots)
				{
					if (slot->getFillX() && slot->getFillY())
					{
						slot->getOwnedWidget()->setComputedSize(glm::vec2(availlableSizeForOneFilled, availlableSizeForFilledY));
						// We update the child computed bounds
						slot->getOwnedWidget()->updateLayer(false);
					}
					else if (slot->getFillX() && !slot->getFillY())
					{
						glm::vec2 slotSize = slot->getOwnedWidget()->getComputedSize();
						slot->getOwnedWidget()->setComputedSize(glm::vec2(availlableSizeForOneFilled, slotSize.y));
						// We update the child computed bounds
						slot->getOwnedWidget()->updateLayer(false);
					}
					else if (!slot->getFillX() && slot->getFillY())
					{
						glm::vec2 slotSize = slot->getOwnedWidget()->getComputedSize();
						slot->getOwnedWidget()->setComputedSize(glm::vec2(slotSize.x, availlableSizeForFilledY));
						// We update the child computed bounds
						slot->getOwnedWidget()->updateLayer(false);
					}
				}
			}

			// Once we have modified the size of childs, we need to update them
			//for (auto& slot : m_slots)
			//{
			//	if(!slot->getSizeToContent())
			//		slot->getOwnedWidget()->updateLayer();
			//}
		}

		// update slots position
		glm::vec2 cursor(0, 0);
		float contentSizeX = 0;
		float contentSizeY = 0;
		for (auto& slot : m_slots)
		{
			slot->getOwnedWidget()->setComputedRelativePosition(cursor);
			cursor.y += slot->getOwnedWidget()->getComputedSize().y;
			contentSizeX = std::max(contentSizeX, slot->getOwnedWidget()->getComputedSize().x);
		}
		contentSizeY = cursor.y;

		// If the current widget is a sizeToContent widget, we update its size based on the content
		if (!ignoreSizeToContent && getOwningWidgetSlot()->getSizeToContent())
		{
			// We update only if the size has changed to avoid circulary calls
			const glm::vec2& computedSize = m_owningWidget->getComputedSize();
			if (computedSize != glm::vec2(contentSizeX, contentSizeY))
			{
				m_owningWidget->setComputedSize(glm::vec2(contentSizeX, contentSizeY));

				if (canUpdateParent)
				{
					// this time we update the parent
					m_owningWidget->getOwningSlot()->getOwningLayer()->updateSlotsRects(false);
				}
			}
		}
	}
};

