#include "Widget.h"
#include "WidgetLayer.h"
#include "UIEngine.h"


WidgetBase::WidgetBase(UIEngine* uiengine)
	: UIItem(uiengine)
{}

//const glm::vec2& WidgetBase::getPosition() const
//{
//	return m_box.pos;
//}
//
//const glm::vec2& WidgetBase::getSize() const
//{
//	return m_box.extent;
//}
//
//void WidgetBase::setSize(const glm::vec2& size, bool recursiveUpdateFromParent)
//{
//	m_box.extent = size;
//}

////////////////////////

Widget::Widget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> program)
	: WidgetBase(uiengine)
	, m_shape(shape)
	, m_program(program)
	, m_owningSlot(nullptr)
	, m_tint(1,1,1,1)
	, m_cornerRadius(0)
{
	setPreferredSize(glm::vec2(50, 50));
}

Widget::~Widget()
{

}

void Widget::draw(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const
{
	// self draw
	if (m_program.expired() || m_shape.expired())
		return;

	// we bind the program only if it is not already in use
	auto program = m_program.lock();
	if (program.get() != *boundProgram)
	{
		program->use();
		*boundProgram = program.get();
	}

	// compute render box
	glm::vec4 box = m_computedBounds.toVec4();
	viewportTransform(box, viewportSize);
	//glm::vec4 box((m_posInViewport / (viewportSize * 0.5f)), m_box.extent / (viewportSize * 0.5f));
	//box *= glm::vec4(1, -1, 1, -1);
	//box += glm::vec4(-1, 1, 0, 0);

	// update uniforms
	glUniform4fv(glGetUniformLocation(program->getGLId(), "box"), 1, &box[0]/*&m_box.toVec4()[0]*/);
	glUniform4fv(glGetUniformLocation(program->getGLId(), "tint"), 1, &getTint()[0]);
	glUniform2fv(glGetUniformLocation(program->getGLId(), "viewportSize"), 1, &viewportSize[0]);
	glUniform1f(glGetUniformLocation(program->getGLId(), "cornerRadius"), m_cornerRadius);

	// draw the widget shape
	auto shape = m_shape.lock();
	shape->draw();

	// recursivity on layer slots
	if (m_layer != nullptr)
		m_layer->draw(boundProgram, viewportSize);
}

bool Widget::isMouseHovering(const glm::vec2& cursor) const
{
	return m_computedBounds.isPointInside(cursor);
}

bool Widget::acceptLayer() const
{
	return true;
}

WidgetSlot* Widget::getOwningSlot() const
{
	return m_owningSlot;
}

BaseWidgetLayer* Widget::getOwningLayer() const
{
	if (m_owningSlot == nullptr)
		return nullptr;
	else
		return m_owningSlot->getOwningLayer();
}

void Widget::setLayer(const std::shared_ptr<BaseWidgetLayer>& layer)
{
	m_layer = layer;
	//for (auto& child : m_childs)
	//{
	//	m_layer->addSlot(child.get());
	//}
	m_layer->setOwningWidget(this);

	updateTransformFromParent();
}

void Widget::removeLayer()
{
	//m_layer->clearSlots();
	m_layer.reset();
	m_layer->setOwningWidget(nullptr);

	updateTransformFromParent();
}

BaseWidgetLayer* Widget::getLayer() const
{
	return m_layer.get();
}
//
//void Widget::addChild(std::shared_ptr<Widget> child)
//{
//	m_childs.push_back(child);
//	child->m_parent = m_parent;
//
//	if (m_layer)
//		m_layer->addSlot(child.get());
//}
//
//void Widget::removeChild(Widget* child)
//{
//	auto found = std::find_if(m_childs.begin(), m_childs.end(), [child](const std::shared_ptr<Widget>& item) { return item.get() == child; });
//	int foundIdx = std::distance(m_childs.begin(), found);
//	if (found != m_childs.end())
//	{
//		m_childs.erase(found);
//
//		if (m_layer)
//			m_layer->removeSlot(foundIdx);
//	}
//}

const std::string& Widget::getName() const
{
	return m_name;
}

//void Widget::setSize(const glm::vec2& size, bool recursiveUpdateFromParent)
//{
//	m_box.extent = size;
//	m_boxInViewport.extent = size;
//
//	if(recursiveUpdateFromParent)
//		updateTransformFromParent();
//}
//
//void Widget::scaleBy(const glm::vec2& scaleFactor, bool recursiveUpdateFromParent)
//{
//	setSize(m_box.extent * scaleFactor);
//}
//
//void Widget::setPosition(const glm::vec2& pos, bool recursiveUpdateFromParent)
//{
//	m_box.pos = pos;
//
//	// update global position
//	computePositionInViewport();
//
//	if (recursiveUpdateFromParent)
//		updateTransformFromParent();
//}
//
//void Widget::addOffset(const glm::vec2& offset, bool recursiveUpdateFromParent)
//{
//	setPosition(getPosition() + offset, recursiveUpdateFromParent);
//}

void Widget::computeChildrenPositionInViewport()
{
	if (m_layer)
		m_layer->computeChildrenPositionInViewport();
}

void Widget::computePositionInViewportRecur()
{
	computePositionInViewport();

	if (m_layer)
		m_layer->computePositionInViewportRecur();
}

void Widget::computePositionInViewport()
{
	if (m_owningSlot != nullptr && m_owningSlot->getOwningLayer() != nullptr
		&& m_owningSlot->getOwningLayer()->isAttachedToWidget())
	{
		m_computedBounds.pos = m_owningSlot->getOwningLayer()->getPosition() + getComputedRelativePosition();
	}
}

void Widget::updateTransformFromParent()
{
	auto owningLayer = getOwningLayer();
	if (owningLayer)
	{
		owningLayer->updateSlotsRecur();
	}
	else
	{
		updateLayer();
	}
}

void Widget::updateLayer(bool canUpdateParent, bool ignoreSizeToContent)
{
	if (m_layer)
		m_layer->updateSlotsRecur(canUpdateParent, ignoreSizeToContent);
}

bool Widget::handleMouseMove(const glm::vec2 mousePos) 
{
	if (m_visibility == WidgetVisibility::INVISILE
		|| m_visibility == WidgetVisibility::COLLAPSED
		|| m_visibility == WidgetVisibility::HIT_TEST_INVISIBLE)
		return false;

	bool handled = m_layer == nullptr ? false : m_layer->handleMouseMove(mousePos);
	if (m_visibility == WidgetVisibility::SELF_HIT_TEST_INVISIBLE)
		return handled;

	if (isMouseHovering(mousePos))
	{
		handled = onMouseMove(mousePos);

		if (!getIsHovered())
		{
			onMouseEnter();
			if (m_uiEngine->isDraggingItem())
				onDragEnter();

			setIsHovered(true);
		}
	}
	else
	{
		if (getIsHovered())
		{
			onMouseLeave();
			if (m_uiEngine->isDraggingItem())
				onDragLeave();

			setIsHovered(false);
		}

		// mouse is not in any child so we don't bubble this event on childs
		handled = true;
	}

	return handled;
}

bool Widget::handleMouseButtonPressed(int button, const glm::vec2& mousePos)
{
	if (m_visibility == WidgetVisibility::INVISILE
		|| m_visibility == WidgetVisibility::COLLAPSED
		|| m_visibility == WidgetVisibility::HIT_TEST_INVISIBLE)
		return false;

	if (isMouseHovering(mousePos))
	{
		if (m_layer == nullptr)
			m_uiEngine->selectItem(this);

		m_uiEngine->addPressedItem(this);

		// we propagate event on childs only if the mouse is inside the parent
		bool handled = (m_layer == nullptr) ? false : m_layer->handleMouseButtonPressed(button, mousePos);

		if (handled)
			return true;
		else if(m_visibility != WidgetVisibility::SELF_HIT_TEST_INVISIBLE)
		{
			handled = onMouseButtonPressed(button, mousePos);
		}
	}
	else
	{
		m_selected = false;
		return false;
	}
}

bool Widget::handleMouseButtonReleased(int button, const glm::vec2& mousePos) 
{
	if (m_visibility == WidgetVisibility::INVISILE
		|| m_visibility == WidgetVisibility::COLLAPSED
		|| m_visibility == WidgetVisibility::HIT_TEST_INVISIBLE)
		return false;

	if (isMouseHovering(mousePos))
	{
		// we propagate event on childs only if the mouse is inside the parent
		bool handled = (m_layer == nullptr) ? false : m_layer->handleMouseButtonReleased(button, mousePos);
		if (handled)
			return true;
		else if(m_visibility != WidgetVisibility::SELF_HIT_TEST_INVISIBLE)
		{
			handled = onMouseButtonReleased(button, mousePos);
		}
	}
	else
		return false;
}

bool Widget::handleKeyPressed(int key)
{
	if (m_visibility == WidgetVisibility::INVISILE
		|| m_visibility == WidgetVisibility::COLLAPSED
		|| m_visibility == WidgetVisibility::HIT_TEST_INVISIBLE)
		return false;

	bool handled = m_layer == nullptr ? false : m_layer->handleKeyPressed(key);
	if (handled)
		return true;

	if (m_visibility != WidgetVisibility::SELF_HIT_TEST_INVISIBLE)
		handled = onKeyPressed(key);

	return handled;
}

bool Widget::handleKeyReleased(int key)
{
	if (m_visibility == WidgetVisibility::INVISILE
		|| m_visibility == WidgetVisibility::COLLAPSED
		|| m_visibility == WidgetVisibility::HIT_TEST_INVISIBLE)
		return false;

	bool handled = m_layer == nullptr ? false : m_layer->handleKeyReleased(key);
	if (handled)
		return true;

	if (m_visibility != WidgetVisibility::SELF_HIT_TEST_INVISIBLE)
		handled = onKeyReleased(key);

	return handled;
}

bool Widget::handleCharacter(unsigned int codepoint)
{
	if (m_visibility == WidgetVisibility::INVISILE
		|| m_visibility == WidgetVisibility::COLLAPSED
		|| m_visibility == WidgetVisibility::HIT_TEST_INVISIBLE)
		return false;

	bool handled = m_layer == nullptr ? false : m_layer->handleCharacter(codepoint);
	if (handled)
		return true;

	if (m_visibility != WidgetVisibility::SELF_HIT_TEST_INVISIBLE)
		handled = onCharacter(codepoint);

	return handled;
}

bool Widget::handleDragOver(const glm::vec2& mousePos)
{
	if (m_visibility == WidgetVisibility::INVISILE
		|| m_visibility == WidgetVisibility::COLLAPSED
		|| m_visibility == WidgetVisibility::HIT_TEST_INVISIBLE)
		return false;

	if (isMouseHovering(mousePos))
	{
		// we propagate event on childs only if the mouse is inside the parent
		bool handled = (m_layer == nullptr) ? false : m_layer->handleDragOver(mousePos);
		if (handled)
			return true;
		else if (m_visibility != WidgetVisibility::SELF_HIT_TEST_INVISIBLE)
		{
			handled = onDragOver(mousePos);
		}
	}
	else
		return false;
}

bool Widget::handleDrop(const glm::vec2& mousePos)
{
	if (m_visibility == WidgetVisibility::INVISILE
		|| m_visibility == WidgetVisibility::COLLAPSED
		|| m_visibility == WidgetVisibility::HIT_TEST_INVISIBLE)
		return false;

	if (isMouseHovering(mousePos))
	{
		// we propagate event on childs only if the mouse is inside the parent
		bool handled = (m_layer == nullptr) ? false : m_layer->handleDrop(mousePos);
		if (handled)
			return true;
		else if (m_visibility != WidgetVisibility::SELF_HIT_TEST_INVISIBLE)
		{
			handled = onDrop(mousePos);
		}
	}
	else
		return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
////// WidgetSlot
/////////////////////////////////////////////////////////////////////////////////////////////////////

WidgetSlot::WidgetSlot(BaseWidgetLayer* owningLayer, std::shared_ptr<Widget> ownedWidget)
	: m_ownedWidget(ownedWidget)
	, m_owningLayer(owningLayer)
	, m_sizeToContent(false)
	, m_padding(0, 0, 0, 0)
{
	ownedWidget->m_owningSlot = this;
}

Widget* WidgetSlot::getOwnedWidget() const
{
	return m_ownedWidget.get();
}
std::shared_ptr<Widget> WidgetSlot::getOwnedWidgetShared() const
{
	return m_ownedWidget;
}
BaseWidgetLayer* WidgetSlot::getOwningLayer() const
{
	return m_owningLayer;
}

bool WidgetSlot::getSizeToContent() const
{
	return m_sizeToContent;
}
const WidgetPadding& WidgetSlot::getPadding() const
{
	return m_padding;
}

void WidgetSlot::setSizeToContent(bool sizeToContent)
{
	if (m_sizeToContent != sizeToContent)
	{
		m_sizeToContent = sizeToContent;

		if (m_owningLayer != nullptr)
			m_owningLayer->updateSlotsRecur();
	}
}
void WidgetSlot::setPadding(const WidgetPadding& padding)
{
	if (m_padding != padding)
	{
		m_padding = padding;

		if (m_owningLayer != nullptr)
			m_owningLayer->updateSlotsRecur();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
////// RawSlot
/////////////////////////////////////////////////////////////////////////////////////////////////////

RawSlot::RawSlot(BaseWidgetLayer* _owningLayer, std::shared_ptr<Widget> _ownedWidget)
	: WidgetSlot(_owningLayer, _ownedWidget)
{}
RawSlot::~RawSlot()
{}

void RawSlot::setSize(const glm::vec2& size)
{
	m_size = size;
	m_ownedWidget->setPreferredSize(size);
}
const glm::vec2& RawSlot::getSize() const
{
	return m_size;
}

void RawSlot::setPosition(const glm::vec2& position)
{
	m_position = position;
	m_ownedWidget->setComputedRelativePosition(m_position);
}
const glm::vec2 RawSlot::getPosition() const
{
	return m_position;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
////// ListSlot
/////////////////////////////////////////////////////////////////////////////////////////////////////

ListSlot::ListSlot(BaseWidgetLayer* _owningLayer, std::shared_ptr<Widget> _ownedWidget)
	: WidgetSlot(_owningLayer, _ownedWidget)
	, m_fillX(false)
	, m_fillY(false)
{}

ListSlot::~ListSlot()
{}

bool ListSlot::getFillX() const
{
	return m_fillX;
}
bool ListSlot::getFillY() const
{
	return m_fillY;
}

void ListSlot::setFillX(bool fillX)
{
	m_fillX = fillX;

	if (m_owningLayer != nullptr)
		m_owningLayer->updateSlotsRecur();
}
void ListSlot::setFillY(bool fillY)
{
	m_fillY = fillY;

	if (m_owningLayer != nullptr)
		m_owningLayer->updateSlotsRecur();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
////// CanvasSlot
/////////////////////////////////////////////////////////////////////////////////////////////////////

CanvasSlot::CanvasSlot(BaseWidgetLayer* _owningLayer, std::shared_ptr<Widget> _ownedWidget)
	: WidgetSlot(_owningLayer, _ownedWidget)
{}

CanvasSlot::~CanvasSlot()
{}

void CanvasSlot::setSize(const glm::vec2& size)
{
	m_size = size;
	m_ownedWidget->setPreferredSize(size);
}

const glm::vec2& CanvasSlot::getSize() const
{
	return m_size;
}

WidgetAnchor& CanvasSlot::getAnchor()
{
	return m_anchor;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
////// ViewportWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////

void ViewportWidget::draw(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const
{
	for (const auto& layer : m_layers)
	{
		layer.second->draw(boundProgram, m_computedBounds.extent);
	}
}

bool ViewportWidget::isMouseHovering(const glm::vec2& cursor) const
{
	return m_computedBounds.isPointInside(cursor);
}

bool ViewportWidget::acceptLayer() const
{
	return true;
}

void ViewportWidget::setViewport(const glm::vec2& pos, const glm::vec2& extent)
{
	setComputedRelativePosition(pos);
	setComputedSize(extent);
	setPreferredSize(extent);

	for (auto& layer : m_layers)
	{
		layer.second->updateSlotsRecur();
	}
}

//void ViewportWidget::setSize(const glm::vec2& size, bool updateChilds)
//{
//	m_box.extent = size;
//
//	if (updateChilds)
//	{
//		for (auto& layer : m_layers)
//		{
//			layer.second->updateSlotsRecur();
//		}
//	}
//}

void ViewportWidget::computePositionInViewportRecur()
{
	for (auto& layer : m_layers)
	{
		layer.second->computePositionInViewportRecur();
	}
}

void ViewportWidget::computeChildrenPositionInViewport()
{
	for (auto& layer : m_layers)
	{
		layer.second->computeChildrenPositionInViewport();
	}
}

void ViewportWidget::computePositionInViewport()
{
	// nothing to do here
}

// layer handling
void ViewportWidget::addLayer(std::shared_ptr<BaseWidgetLayer> layer, int zorder)
{
	m_layers.insert(std::pair<int, std::shared_ptr<BaseWidgetLayer>>(zorder, layer));
	layer->setOwningWidget(this);

	layer->updateSlotsRecur();
}

void ViewportWidget::removeLayer(std::shared_ptr<BaseWidgetLayer> layer)
{
	auto found = std::find_if(m_layers.begin(), m_layers.end(), [&layer](const std::pair<int, std::shared_ptr<BaseWidgetLayer>>& item) { return item.second.get() == layer.get(); });
	if (found != m_layers.end())
	{
		m_layers.erase(found);
	}
}

bool ViewportWidget::handleMouseMove(const glm::vec2 mousePos) 
{
	bool handled = false;
	if (isMouseHovering(mousePos))
	{
		for (auto& layer : m_layers)
		{
			bool handledByChild = layer.second->handleMouseMove(mousePos);
			handled = handled || handledByChild;
		}
	}
	else
	{
		// mouse is not in any child so we don't bubble this event on childs
		handled = true;
	}

	return handled;
}

bool ViewportWidget::handleMouseButtonPressed(int button, const glm::vec2& mousePos)
{
	bool handled = false;
	if (isMouseHovering(mousePos))
	{
		for (auto& layer : m_layers)
		{
			bool handledByChild = layer.second->handleMouseButtonPressed(button, mousePos);
			handled = handled || handledByChild;
		}
	}
	else
	{
		// mouse is not in any child so we don't bubble this event on childs
		handled = true;
	}

	return handled;
}

bool ViewportWidget::handleMouseButtonReleased(int button, const glm::vec2& mousePos)
{
	bool handled = false;
	if (isMouseHovering(mousePos))
	{
		for (auto& layer : m_layers)
		{
			bool handledByChild = layer.second->handleMouseButtonReleased(button, mousePos);
			handled = handled || handledByChild;
		}
	}
	else
	{
		// mouse is not in any child so we don't bubble this event on childs
		handled = true;
	}

	return handled;
}

bool ViewportWidget::handleKeyPressed(int key)
{
	bool handled = false;

	for (auto& layer : m_layers)
	{
		bool handledByChild = layer.second->handleKeyPressed(key);
		handled = handled || handledByChild;
	}

	return handled;
}

bool ViewportWidget::handleKeyReleased(int key)
{
	bool handled = false;

	for (auto& layer : m_layers)
	{
		bool handledByChild = layer.second->handleKeyReleased(key);
		handled = handled || handledByChild;
	}

	return handled;
}

bool ViewportWidget::handleCharacter(unsigned int codepoint)
{
	bool handled = false;

	for (auto& layer : m_layers)
	{
		bool handledByChild = layer.second->handleCharacter(codepoint);
		handled = handled || handledByChild;
	}

	return handled;
}

///////////////////////////////////////////////////////////////////////

ImageWidget::ImageWidget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> program)
	: Widget(uiengine, shape, program)
{

}

ImageWidget::~ImageWidget()
{

}

void ImageWidget::setTexture(std::shared_ptr<Texture> texture)
{
	m_texture = texture;
}

const Texture* ImageWidget::getTexture() const
{
	return m_texture.get();
}

void ImageWidget::draw(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const
{
	// self draw
	if (m_program.expired() || m_shape.expired() || m_visibility == WidgetVisibility::INVISILE	|| m_visibility == WidgetVisibility::COLLAPSED)
		return;

	// we bind the program only if it is not already in use
	auto program = m_program.lock();
	if (program.get() != *boundProgram)
	{
		program->use();
		*boundProgram = program.get();
	}

	// compute render box
	glm::vec4 box = m_computedBounds.toVec4();
	viewportTransform(box, viewportSize);
	//box((m_posInViewport / (viewportSize * 0.5f)), m_box.extent / (viewportSize * 0.5f));
	//box *= glm::vec4(1, -1, 1, -1);
	//box += glm::vec4(-1, 1, 0, 0);

	// activate texture unit and bind texture
	glActiveTexture(GL_TEXTURE0);
	m_texture->bind();

	// update uniforms
	glUniform4fv(glGetUniformLocation(program->getGLId(), "box"), 1, &box[0]/*&m_box.toVec4()[0]*/);
	glUniform4fv(glGetUniformLocation(program->getGLId(), "tint"), 1, &getTint()[0]);
	glUniform2fv(glGetUniformLocation(program->getGLId(), "viewportSize"), 1, &viewportSize[0]);
	glUniform1f(glGetUniformLocation(program->getGLId(), "cornerRadius"), m_cornerRadius);

	// draw the widget shape
	auto shape = m_shape.lock();
	shape->draw();

	// unbind texture
	m_texture->unbind();

	// recursivity on layer slots
	if (m_layer != nullptr)
		m_layer->draw(boundProgram, viewportSize);
}


//////////////////////////////////////////////////////////////////////////////////////


TextWidget::TextWidget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> program)
	: Widget(uiengine, shape, program)
{

}

TextWidget::~TextWidget()
{

}

void TextWidget::setFont(std::shared_ptr<Font> font)
{
	m_font = font;
}

const Font* TextWidget::getFont() const
{
	return m_font.get();
}

void TextWidget::setText(const std::string& text)
{
	m_text = text;

	assert(m_font && "set a font before modifying the text.");

	m_textBounds = m_font->computeTextBounds(m_text);
	setPreferredSize(m_textBounds.extent);
}

const std::string& TextWidget::getText() const
{
	return m_text;
}

void TextWidget::draw(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const
{
	// self draw
	if (m_program.expired() || m_shape.expired() || m_visibility == WidgetVisibility::INVISILE || m_visibility == WidgetVisibility::COLLAPSED)
		return;

	drawText(boundProgram, viewportSize);

	// recursivity on layer slots
	if (m_layer != nullptr)
		m_layer->draw(boundProgram, viewportSize);
}

void TextWidget::drawText(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const
{
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	// we bind the program only if it is not already in use
	auto program = m_program.lock();
	if (program.get() != *boundProgram)
	{
		program->use();
		*boundProgram = program.get();
	}

	// activate texture unit and bind texture
	glActiveTexture(GL_TEXTURE0);
	m_font->bindTexture();

	glm::vec2 cursor = m_computedBounds.pos + glm::vec2(0, m_textBounds.extent.y);
	glm::vec2 nextCursor(0, 0);
	glm::vec4 glyphSrcRect;
	glm::vec4 glyphDstRect;

	for (const auto& character : m_text)
	{
		m_font->getGlyphDisplayInfos(character, cursor, nextCursor, glyphDstRect, glyphSrcRect);

		// compute render box
		glyphDstRect /= glm::vec4((viewportSize), (viewportSize));
		glyphDstRect *= glm::vec4(2, -2, 2, -2);
		glyphDstRect += glm::vec4(-1, 1, 0, 0);

		// update uniforms
		glUniform4fv(glGetUniformLocation(program->getGLId(), "box"), 1, &glyphDstRect[0]);
		glUniform4fv(glGetUniformLocation(program->getGLId(), "glyphSrcRect"), 1, &glyphSrcRect[0]);
		glUniform4fv(glGetUniformLocation(program->getGLId(), "tint"), 1, &getTint()[0]);
		glUniform2fv(glGetUniformLocation(program->getGLId(), "viewportSize"), 1, &viewportSize[0]);

		// draw the widget shape
		auto shape = m_shape.lock();
		shape->draw();

		cursor = nextCursor;
	}

	// unbind texture
	m_font->unbindTexture();

	glPixelStorei(GL_PACK_ALIGNMENT, 4);
}



//////////////////////////////////////////////////////////////////////////////////////


TextInputWidget::TextInputWidget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> textProgram, std::weak_ptr<ShaderProgram> cursorProgram)
	: Widget(uiengine, shape, textProgram)
	, m_cursorPos(0)
	, m_cursorProgram(cursorProgram)
{

}

TextInputWidget::~TextInputWidget()
{

}

void TextInputWidget::setFont(std::shared_ptr<Font> font)
{
	m_font = font;
}

const Font* TextInputWidget::getFont() const
{
	return m_font.get();
}

void TextInputWidget::setText(const std::string& text)
{
	m_text = text;

	m_textBounds = m_font->computeTextBounds(m_text);
	setPreferredSize(m_textBounds.extent);
}

const std::string& TextInputWidget::getText() const
{
	return m_text;
}

void TextInputWidget::addCharacter(char character)
{
	m_text.insert(m_text.begin() + m_cursorPos, character);

	m_textBounds = m_font->computeTextBounds(m_text);
	setPreferredSize(m_textBounds.extent);

	cursorNext();
}

void TextInputWidget::removePreviousCharacter()
{
	if (m_cursorPos > 0)
	{
		m_text.erase(m_text.begin() + (m_cursorPos - 1));

		m_textBounds = m_font->computeTextBounds(m_text);
		setPreferredSize(m_textBounds.extent);

		cursorPrevious();
	}
}

void TextInputWidget::removeNextCharacter()
{
	if (m_cursorPos < m_text.size())
	{
		m_text.erase(m_text.begin() + m_cursorPos);

		m_textBounds = m_font->computeTextBounds(m_text);
		setPreferredSize(m_textBounds.extent);

		//cursorPrevious();
	}
}

void TextInputWidget::cursorNext()
{
	m_cursorPos = std::min(std::max(0, m_cursorPos + 1), (int)(m_text.size()));
}

void TextInputWidget::cursorPrevious()
{
	m_cursorPos = std::min(std::max(0, m_cursorPos - 1), (int)(m_text.size()));
}

void TextInputWidget::draw(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const
{
	// self draw
	if (m_program.expired() || m_shape.expired() || m_visibility == WidgetVisibility::INVISILE || m_visibility == WidgetVisibility::COLLAPSED)
		return;

	drawText(boundProgram, viewportSize);
	if(getIsSelected())
		drawCursor(boundProgram, viewportSize);

	// recursivity on layer slots
	if (m_layer != nullptr)
		m_layer->draw(boundProgram, viewportSize);
}

void TextInputWidget::drawText(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const
{
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	// we bind the program only if it is not already in use
	auto program = m_program.lock();
	if (program.get() != *boundProgram)
	{
		program->use();
		*boundProgram = program.get();
	}

	// activate texture unit and bind texture
	glActiveTexture(GL_TEXTURE0);
	m_font->bindTexture();

	glm::vec2 cursor = m_computedBounds.pos + glm::vec2(0, m_textBounds.extent.y);
	glm::vec2 nextCursor(0, 0);
	glm::vec4 glyphSrcRect;
	glm::vec4 glyphDstRect;

	for (const auto& character : m_text)
	{
		m_font->getGlyphDisplayInfos(character, cursor, nextCursor, glyphDstRect, glyphSrcRect);

		// compute render box
		glyphDstRect /= glm::vec4((viewportSize), (viewportSize));
		glyphDstRect *= glm::vec4(2, -2, 2, -2);
		glyphDstRect += glm::vec4(-1, 1, 0, 0);

		// update uniforms
		glUniform4fv(glGetUniformLocation(program->getGLId(), "box"), 1, &glyphDstRect[0]);
		glUniform4fv(glGetUniformLocation(program->getGLId(), "glyphSrcRect"), 1, &glyphSrcRect[0]);
		glUniform4fv(glGetUniformLocation(program->getGLId(), "tint"), 1, &getTint()[0]);
		glUniform2fv(glGetUniformLocation(program->getGLId(), "viewportSize"), 1, &viewportSize[0]);

		// draw the widget shape
		auto shape = m_shape.lock();
		shape->draw();

		cursor = nextCursor;
	}

	// unbind texture
	m_font->unbindTexture();

	glPixelStorei(GL_PACK_ALIGNMENT, 4);
}


void TextInputWidget::drawCursor(ShaderProgram** boundProgram, const glm::vec2& viewportSize) const
{
	// we bind the program only if it is not already in use
	auto program = m_cursorProgram.lock();
	if (program.get() != *boundProgram)
	{
		program->use();
		*boundProgram = program.get();
	}


	// compute render box
	glm::vec4 box(	m_computedBounds.pos + m_font->getCursorPos(m_text, m_cursorPos), glm::vec2(4, m_font->getMaxGlyphSize().y) );
	viewportTransform(box, viewportSize);

	// update uniforms
	glUniform4fv(glGetUniformLocation(program->getGLId(), "box"), 1, &box[0]);
	glUniform4fv(glGetUniformLocation(program->getGLId(), "tint"), 1, &getTint()[0]);
	glUniform2fv(glGetUniformLocation(program->getGLId(), "viewportSize"), 1, &viewportSize[0]);

	// draw the widget shape
	auto shape = m_shape.lock();
	shape->draw();
}


////////////////////////////////////////////////////////////////////////////////

DropDownWidget::DropDownWidget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> textProgram)
	: Widget(uiengine, shape, textProgram)
	, m_emptyItemListText("...")
{
	m_dropDownListLayout = uiengine->instantiateLayer("Raw");
	auto dropDownList = uiengine->instantiateWidget("EmptyWidget");
	auto dropDownListContent = uiengine->instantiateLayer("VerticalList");
	dropDownList->setLayer(dropDownListContent);
	auto dropDownListSlot = m_dropDownListLayout->addSlot(dropDownList);
	dropDownListSlot->setSizeToContent(true);
	dropDownList->setVisibility(WidgetVisibility::COLLAPSED);

	m_dropDownListContent = dropDownListContent.get();
	m_dropDownList = dropDownList.get();

	auto selection = uiengine->instantiateWidgetAs<ButtonWidget>("ButtonWidget");
	selection->setLayer(uiengine->instantiateLayer("Canvas"));
	auto selectionText = uiengine->instantiateWidgetAs<TextWidget>("TextWidget");
	selectionText->setFont(m_uiEngine->getFontFactory().getDefaultFont());
	selectionText->setText(m_emptyItemListText);
	selection->getLayer()->addSlot(selectionText);

	m_selection = selection.get();
	m_selectionText = selectionText.get();

	selection->onClicked = [this](int button, const glm::vec2& mousePos) { this->toggleDropDownListVisibility(); return true; };

	setLayer(uiengine->instantiateLayer("Canvas"));
	getLayer()->addSlot(selection);
}

DropDownWidget::~DropDownWidget()
{}

// drop down list
void DropDownWidget::displayDropDownList()
{
	m_uiEngine->getRootViewportWidget()->addLayer(m_dropDownListLayout, 10);

	m_dropDownList->setComputedRelativePosition(m_selection->getComputedPosition() + glm::vec2(0, m_selection->getComputedBounds().extent.y));
	m_dropDownList->updateLayer();
	m_dropDownList->setVisibility(WidgetVisibility::VISIBLE);
}

void DropDownWidget::hideItemDropDown()
{
	m_uiEngine->getRootViewportWidget()->removeLayer(m_dropDownListLayout);

	m_dropDownList->setVisibility(WidgetVisibility::COLLAPSED);
}

void DropDownWidget::toggleDropDownListVisibility()
{
	if (m_dropDownList->getVisibility() == WidgetVisibility::COLLAPSED)
		displayDropDownList();
	else
		hideItemDropDown();
}

void DropDownWidget::onItemUnselected()
{
	hideItemDropDown();
}

// option items
std::shared_ptr<Widget> DropDownWidget::createDefaultItem(const std::string& label, int indexInList)
{
	auto newItem = m_uiEngine->instantiateWidgetAs<ButtonWidget>("ButtonWidget");
	newItem->setLayer(m_uiEngine->instantiateLayer("Canvas"));
	auto text = m_uiEngine->instantiateWidgetAs<TextWidget>("TextWidget");
	text->setFont(m_uiEngine->getFontFactory().getDefaultFont());
	text->setText(label);
	auto slot = newItem->getLayer()->addSlot(text);
	slot->setSizeToContent(true);

	newItem->onClicked = [this, indexInList](int button, const glm::vec2& mousePos) { this->selectItem(indexInList); return true; };

	return newItem;
}
void DropDownWidget::addDefaultItem(const std::string& label)
{
	const int indexInList = m_labels.size();

	auto newItem = createDefaultItem(label, indexInList);
	auto slot = m_dropDownList->getLayer()->addSlot(newItem);
	slot->setSizeToContent(true);
	m_labels.push_back(label);

	if (m_labels.size() == 1)
		selectItem(0);
}
void DropDownWidget::clearItems()
{
	m_dropDownList->getLayer()->clearSlots();
	m_labels.clear();
	selectItem(-1);
}
void DropDownWidget::setDefaultItems(const std::vector<std::string>& labels)
{
	clearItems();
	int index = 0;
	for (const auto& label : labels)
	{
		addDefaultItem(label);
	}
}
void DropDownWidget::selectItem(int itemIdx)
{
	if (itemIdx < 0 || itemIdx >= m_labels.size())
	{
		m_selectionText->setText(m_emptyItemListText);
		m_currentSelectedItem = -1;
	}

	m_selectionText->setText(m_labels[itemIdx]);
	m_currentSelectedItem = itemIdx;
}
const std::string& DropDownWidget::getSelectedItemLabel() const
{
	if (m_currentSelectedItem < 0 || m_currentSelectedItem >= m_labels.size())
		return m_emptyItemListText;
	else
		return m_labels[m_currentSelectedItem];
}
int DropDownWidget::getSelectedItemIdx() const
{
	return m_currentSelectedItem;
}

///////////////////////////////////////////////////////////////////////////
/////// WindowWidget
///////////////////////////////////////////////////////////////////////////


WindowWidget::WindowWidget(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> shaderProgram)
	: Widget(uiengine, shape, shaderProgram)
{
	auto backgroundLayout = uiengine->instantiateLayer("VerticalList");
	setLayer(backgroundLayout);

	// title
	{
		auto titleBar = uiengine->instantiateWidget("EmptyWidget");
		auto titleBarLayout = uiengine->instantiateLayer("HorizontalList");
		auto titleText = uiengine->instantiateWidgetAs<TextWidget>("TextWidget");
		titleBarLayout->addSlot(titleText);
		titleText->setText("Title");

		auto titleBarSlot = backgroundLayout->addSlotAs<ListSlot>(titleBar);
		titleBarSlot->setFillX(true);
		titleBarSlot->setFillY(false);

		titleBar->setTint(glm::vec4(1, 0, 0, 1)); // TODO

		titleBar->dragBeginCallback = [this](const glm::vec2& mousePos) { this->m_dragOffset = (mousePos - this->getComputedPosition()); };
		titleBar->dragEndCallback = [this](const glm::vec2& mousePos) { this->setComputedRelativePosition(mousePos - this->m_dragOffset); };

		m_titleText = titleText.get();
		m_titleBar = titleBar.get();
	}
	// menu
	{
		auto menu = uiengine->instantiateWidget("EmptyWidget");
		auto menuLayout = uiengine->instantiateLayer("HorizontalList");

		auto menuSlot = backgroundLayout->addSlotAs<ListSlot>(menu);
		menuSlot->setFillX(true);
		menuSlot->setFillY(false);

		m_menu = menu.get();
		m_menuLayout = menuLayout.get();
	}
	// content
	{
		auto content = std::make_shared<WindowFrameSlot>(true, m_uiEngine, m_shape, m_program);

		auto contentSlot = backgroundLayout->addSlotAs<ListSlot>(content);
		contentSlot->setFillX(true);
		contentSlot->setFillY(true);

		content->setTint(glm::vec4(0, 0.5, 0.5, 1)); //TODO

		m_content = content.get();
	}
}
WindowWidget::~WindowWidget()
{}

// title
void WindowWidget::setTitleText(const std::string& title)
{
	m_titleText->setText(title);
}
const std::string& WindowWidget::getTitleText() const
{
	return m_titleText->getText();
}
void WindowWidget::setDisplayTitleBar(bool dislayTitleBar)
{
	if (dislayTitleBar)
		m_titleBar->setVisibility(WidgetVisibility::VISIBLE);
	else
		m_titleBar->setVisibility(WidgetVisibility::COLLAPSED);
}

// menu
void WindowWidget::setDisplayMenu(bool displayMenu)
{
	if (displayMenu)
		m_menu->setVisibility(WidgetVisibility::VISIBLE);
	else
		m_menu->setVisibility(WidgetVisibility::COLLAPSED);
}
void WindowWidget::addMenuItem(std::shared_ptr<Widget> item)
{
	m_menuLayout->addSlot(item);
}
void WindowWidget::removeMenuItem(const Widget* item)
{
	m_menuLayout->removeSlot(item);
}

void WindowWidget::getAllFramesRecurShared(std::vector<std::shared_ptr<WindowFrame>>& outFrames)
{
	if (m_content != nullptr)
		m_content->getAllFramesRecurShared(outFrames);
}

///////////////////////////////////////////////////////////////////////////
/////// WindowFrameSlotContent
///////////////////////////////////////////////////////////////////////////

WindowFrameSlotContent::WindowFrameSlotContent(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> shaderProgram)
	: Widget(uiengine, shape, shaderProgram)
{}
WindowFrameSlotContent::~WindowFrameSlotContent()
{}

void WindowFrameSlotContent::setWindowSlot(WindowFrameSlot* windowSlot)
{
	m_windowFrameSlot = windowSlot;
}

///////////////////////////////////////////////////////////////////////////
/////// WindowFrameSlot
///////////////////////////////////////////////////////////////////////////

WindowFrameSlot::WindowFrameSlot(bool isHorizontal, UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> shaderProgram)
	: WindowFrameSlotContent(uiengine, shape, shaderProgram)
	, m_isHorizontalLayer(isHorizontal)
{
	setLayer(m_uiEngine->instantiateLayer("Canvas"));

	// frame container childs
	{
		// drop layout
		{
			auto dropWidget = uiengine->instantiateWidget("EmptyWidget");
			dropWidget->setLayer(uiengine->instantiateLayer("Canvas"));
			// drop anchor
			{
				auto dropAnchor = uiengine->instantiateWidget("EmptyWidget");
				auto dropAnchorSlot = dropWidget->getLayer()->addSlotAs<CanvasSlot>(dropAnchor);
				dropAnchorSlot->setSize(glm::vec2(200, 200));
				dropAnchorSlot->getAnchor().hasProportionalScale = false;
				dropAnchorSlot->getAnchor().anchorPosition = glm::vec2(0.5, 0.5);
				dropAnchorSlot->getAnchor().pivot = glm::vec2(0.5, 0.5);

				dropAnchor->dropCallback = [this](const glm::vec2& mousePos) { this->handleWindowDrop(); return true; };

				m_dropAnchor = dropAnchor.get();
			} // end drop enchor
			getLayer()->addSlot(dropWidget);

			dropWidget->dragEnterCallback = [this]() { this->displayDropAnchor(); return true; };
			dropWidget->dragLeaveCallback = [this]() { this->HideDropAnchor(); return true; };
		}// end drop layout
		 // content
		{
			auto content = uiengine->instantiateWidget("EmptyWidget");
			if (m_isHorizontalLayer)
				content->setLayer(m_uiEngine->instantiateLayer("HorizontalList"));
			else
				content->setLayer(m_uiEngine->instantiateLayer("VerticalList"));

			getLayer()->addSlot(content);

			m_content = content.get();
		} // end content
	} // end container childs
}

WindowFrameSlot::~WindowFrameSlot()
{}

bool WindowFrameSlot::isSlotEmpty() const
{
	return (getLayer()->getSlotCount() == 0);
}

int WindowFrameSlot::getWidgetChildCount() const
{
	if (!m_layer)
		return 0;
	return m_layer->getSlotCount();
}
WindowFrameSlotContent* WindowFrameSlot::getWidgetChildAsFrameSlotContent(int childIndex)
{
	if (childIndex >= 0 && childIndex < getWidgetChildCount())
		return static_cast<WindowFrameSlotContent*>(m_layer->getSlot(childIndex)->getOwnedWidget());
	else
		return nullptr;
}
std::shared_ptr<WindowFrameSlotContent> WindowFrameSlot::getWidgetChildSharedAsFrameSlotContent(int childIndex)
{
	if (childIndex >= 0 && childIndex < getWidgetChildCount())
		return std::static_pointer_cast<WindowFrameSlotContent>(m_layer->getSlot(childIndex)->getOwnedWidgetShared());
	else
		return nullptr;
}

void WindowFrameSlot::handleWindowDrop()
{
	UIItem* draggedItem = m_uiEngine->getDraggedItem();
	WindowWidget* draggedWindow = dynamic_cast<WindowWidget*>(draggedItem);
	if (draggedWindow != nullptr)
	{
		addWindowFrames(draggedWindow);
	}
}
void WindowFrameSlot::addWindowFrames(WindowWidget* window)
{
	std::vector<std::shared_ptr<WindowFrame>> frames;
	window->getAllFramesRecurShared(frames);

	if (frames.size() > 0)
		addFirstFrame(frames[0]);

	for (int i = 1; i < frames.size(); i++)
	{
		frames[0]->addContent(frames[i]->getContentShared());
	}
}
void WindowFrameSlot::addFirstFrame(const std::shared_ptr<WindowFrame>& frameToAdd)
{
	frameToAdd->setWindowSlot(this);
	ListSlot* slot = getLayer()->addSlotAs<ListSlot>(frameToAdd);
	slot->setFillX(true);
	slot->setFillY(true);
}

void WindowFrameSlot::splitAndAdd(int indexOffset, bool requetedAlignmentIsHorizontal, const WindowFrame* childFrame, const std::shared_ptr<WindowFrameSlot>& frameSlotToAdd)
{
	std::vector<std::shared_ptr<WindowFrameSlotContent>> widgetsToAdd;

	if (frameSlotToAdd->getWidgetChildCount() == 1)
		widgetsToAdd.push_back(frameSlotToAdd->getWidgetChildSharedAsFrameSlotContent(0));
	else if ((requetedAlignmentIsHorizontal == m_isHorizontalLayer)
		&& ((m_isHorizontalLayer && frameSlotToAdd->m_isHorizontalLayer) || (!m_isHorizontalLayer && !frameSlotToAdd->m_isHorizontalLayer)))
	{
		for (int i = 0; i < frameSlotToAdd->getWidgetChildCount(); i++)
		{
			widgetsToAdd.push_back(frameSlotToAdd->getWidgetChildSharedAsFrameSlotContent(i));
		}
	}
	else
	{
		widgetsToAdd.push_back(frameSlotToAdd);
	}

	int frameIndex = -1;
	for (int i = 0; i < getLayer()->getSlotCount(); i++)
	{
		if (getLayer()->getSlot(i)->getOwnedWidget() == childFrame)
			frameIndex = i;
	}
	assert(childFrame >= 0);

	if (requetedAlignmentIsHorizontal == m_isHorizontalLayer)
	{
		for (int i = widgetsToAdd.size() - 1; i <= 0; i--)
		{
			ListSlot* slot = getLayer()->insertSlotAs<ListSlot>(widgetsToAdd[i], frameIndex + indexOffset);
			slot->setFillX(true);
			slot->setFillY(true);
			widgetsToAdd[i]->setWindowSlot(this);
		}
	}
	else
	{
		std::shared_ptr<Widget> savedChild = getLayer()->getSlot(frameIndex)->getOwnedWidgetShared();
		getLayer()->removeSlot(frameIndex);
		auto newChild = std::make_shared<WindowFrameSlot>(requetedAlignmentIsHorizontal, m_uiEngine, m_shape, m_program);
		getLayer()->insertSlot(newChild, frameIndex);

		//////////////////
		ListSlot* slot = newChild->getLayer()->addSlotAs<ListSlot>(savedChild);

		// here the size should always be 1
		for (int i = widgetsToAdd.size() - 1; i <= 0; i--)
		{
			slot = newChild->getLayer()->insertSlotAs<ListSlot>(widgetsToAdd[i], frameIndex + indexOffset);
			slot->setFillX(true);
			slot->setFillY(true);
			widgetsToAdd[i]->setWindowSlot(this);
		}
		//////////////////
	}
}

void WindowFrameSlot::splitAddRight(const WindowFrame* childFrame, const std::shared_ptr<WindowFrameSlot>& frameSlotToAdd)
{
	splitAndAdd(1, true, childFrame, frameSlotToAdd);
}
void WindowFrameSlot::splitAddLeft(const WindowFrame* childFrame, const std::shared_ptr<WindowFrameSlot>& frameSlotToAdd)
{
	splitAndAdd(0, true, childFrame, frameSlotToAdd);
}
void WindowFrameSlot::splitAddTop(const WindowFrame* childFrame, const std::shared_ptr<WindowFrameSlot>& frameSlotToAdd)
{
	splitAndAdd(0, false, childFrame, frameSlotToAdd);
}
void WindowFrameSlot::splitAddBottom(const WindowFrame* childFrame, const std::shared_ptr<WindowFrameSlot>& frameSlotToAdd)
{
	splitAndAdd(1, false, childFrame, frameSlotToAdd);
}

void WindowFrameSlot::displayDropAnchor()
{
	m_dropAnchor->setVisibility(WidgetVisibility::VISIBLE);
}
void WindowFrameSlot::HideDropAnchor()
{
	m_dropAnchor->setVisibility(WidgetVisibility::COLLAPSED);
}

void WindowFrameSlot::getAllFramesRecurShared(std::vector<std::shared_ptr<WindowFrame>>& outFrames)
{
	for (int i = 0; i < m_content->getLayer()->getSlotCount(); i++)
	{
		WidgetSlot* slot = m_content->getLayer()->getSlot(i);
		std::shared_ptr<WindowFrame> widgetAsWindowFrame = std::dynamic_pointer_cast<WindowFrame>(slot->getOwnedWidgetShared());
		if (widgetAsWindowFrame)
		{
			outFrames.push_back(widgetAsWindowFrame);
		}
		else
		{
			std::shared_ptr<WindowFrameSlot> widgetAsWindowFrameSlot = std::dynamic_pointer_cast<WindowFrameSlot>(slot->getOwnedWidgetShared());
			if (widgetAsWindowFrameSlot)
			{
				getAllFramesRecurShared(outFrames);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
/////// WindowFrame
///////////////////////////////////////////////////////////////////////////

WindowFrame::WindowFrame(UIEngine* uiengine, std::weak_ptr<VAO> shape, std::weak_ptr<ShaderProgram> shaderProgram)
	: WindowFrameSlotContent(uiengine, shape, shaderProgram)
{
	auto backgroundLayout = uiengine->instantiateLayer("Canvas");
	setLayer(backgroundLayout);

	// main container
	{
		auto mainContainer = uiengine->instantiateWidget("EmptyWidget");
		mainContainer->setLayer(uiengine->instantiateLayer("VerticalList"));

		// main container childs
		{
			// tabs container
			{
				auto tabContainer = uiengine->instantiateWidget("EmptyWidget");
				tabContainer->setLayer(uiengine->instantiateLayer("HorizontalList"));

				auto tabContainerSlot = mainContainer->getLayer()->addSlotAs<ListSlot>(tabContainer);
				//tabContainerSlot->setSizeToContent(true);
				tabContainerSlot->setFillX(true);
				tabContainerSlot->setFillY(false);
				m_tabsContainer = tabContainer.get();
			}

			// frame container
			{
				auto frameContainer = uiengine->instantiateWidget("EmptyWidget");
				frameContainer->setLayer(uiengine->instantiateLayer("Canvas"));

				// frame container childs
				{
					// drop layout
					{
						auto dropWidget = uiengine->instantiateWidget("EmptyWidget");
						dropWidget->setLayer(uiengine->instantiateLayer("Canvas"));
						// drop anchor
						{
							auto dropAnchor = uiengine->instantiateWidget("EmptyWidget");
							auto dropAnchorSlot = dropWidget->getLayer()->addSlotAs<CanvasSlot>(dropAnchor);
							dropAnchorSlot->setSize(glm::vec2(200, 200));
							dropAnchorSlot->getAnchor().hasProportionalScale = false;
							dropAnchorSlot->getAnchor().anchorPosition = glm::vec2(0.5, 0.5);
							dropAnchorSlot->getAnchor().pivot = glm::vec2(0.5, 0.5);

							dropAnchor->dropCallback = [this](const glm::vec2& mousePos) { this->handleWindowDropMiddle(); return true; };

							m_dropAnchor = dropAnchor.get();
						} // end drop enchor
						frameContainer->getLayer()->addSlot(dropWidget);

						dropWidget->dragEnterCallback = [this]() { this->displayDropAnchor(); return true; };
						dropWidget->dragLeaveCallback = [this]() { this->HideDropAnchor(); return true; };
					}// end drop layout
					 // content
					{
						auto frameContent = uiengine->instantiateWidget("EmptyWidget");
						frameContainer->getLayer()->addSlot(frameContent);

						m_frameContent = frameContent;
					} // end content
				} // end container childs

				auto frameContainerSlot = mainContainer->getLayer()->addSlotAs<ListSlot>(frameContainer);
				frameContainerSlot->setFillX(true);
				frameContainerSlot->setFillY(true);
				m_frameContainer = frameContainer.get();
			} // end container
		}

		backgroundLayout->addSlot(mainContainer);
		m_mainContainer = mainContainer;
	}
}
WindowFrame::~WindowFrame()
{
	m_mainContainer.reset();
	m_frameContent.reset();
}

void WindowFrame::addContent(const std::shared_ptr<Widget>& content)
{
	content->setVisibility(WidgetVisibility::COLLAPSED);
	m_frameContent->getLayer()->addSlot(content);

	auto tabText = m_uiEngine->instantiateWidget("TextWidget");
	auto tabBtn = m_uiEngine->instantiateWidget("ButtonWidget");
	tabBtn->setLayer(m_uiEngine->instantiateLayer("Canvas"));
	tabBtn->getLayer()->addSlot(tabText);

	auto tabBtnSlot = m_tabsContainer->getLayer()->addSlotAs<ListSlot>(tabBtn);
	tabBtnSlot->setFillX(false);
	tabBtnSlot->setFillY(false);

	tabBtn->setTint(glm::vec4(1, 1, 0, 1)); // TODO

	displayContent(m_selectedTab);
}

void WindowFrame::displayContent(int tabIndex)
{
	WidgetSlot* slot = m_frameContent->getLayer()->getSlot(m_selectedTab);
	if (slot != nullptr)
	{
		slot->getOwnedWidget()->setVisibility(WidgetVisibility::COLLAPSED);
	}

	slot = m_frameContent->getLayer()->getSlot(tabIndex);
	if (slot != nullptr)
	{
		slot->getOwnedWidget()->setVisibility(WidgetVisibility::VISIBLE);
		m_selectedTab = tabIndex;
	}
	else
		m_selectedTab = 0;
}

void WindowFrame::addWindowFrames(WindowWidget* window)
{
	std::vector<std::shared_ptr<WindowFrame>> frames;
	window->getAllFramesRecurShared(frames);

	for (auto& frame : frames)
	{
		//if (m_WindowSlot->isSlotEmpty())
		//	m_WindowSlot->addFirstFrame(frame);
		//else
		addContent(frame->getContentShared());
	}
}
void WindowFrame::handleWindowDropMiddle()
{
	UIItem* draggedItem = m_uiEngine->getDraggedItem();
	WindowWidget* draggedWindow = dynamic_cast<WindowWidget*>(draggedItem);
	if (draggedWindow != nullptr)
	{
		addWindowFrames(draggedWindow);
	}
}
void WindowFrame::handleWindowDropRight(const std::shared_ptr<WindowFrameSlot>& frameSlot)
{
	m_WindowSlot->splitAddRight(this, frameSlot);
}
void WindowFrame::handleWindowDropLeft(const std::shared_ptr<WindowFrameSlot>& frameSlot)
{
	m_WindowSlot->splitAddLeft(this, frameSlot);
}
void WindowFrame::handleWindowDropTop(const std::shared_ptr<WindowFrameSlot>& frameSlot)
{
	m_WindowSlot->splitAddTop(this, frameSlot);
}
void WindowFrame::handleWindowDropBottom(const std::shared_ptr<WindowFrameSlot>& frameSlot)
{
	m_WindowSlot->splitAddBottom(this, frameSlot);
}

void WindowFrame::setWindowSlot(WindowFrameSlot* slot)
{
	m_WindowSlot = slot;
}

void WindowFrame::displayDropAnchor()
{
	m_dropAnchor->setVisibility(WidgetVisibility::VISIBLE);
}
void WindowFrame::HideDropAnchor()
{
	m_dropAnchor->setVisibility(WidgetVisibility::COLLAPSED);
}

std::shared_ptr<Widget> WindowFrame::getContentShared() const
{
	return m_frameContent;
}