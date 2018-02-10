#include "WidgetLayer.h"

void BaseWidgetLayer::setOwningWidget(WidgetBase* owningWidget)
{
	m_owningWidget = owningWidget;
}

bool BaseWidgetLayer::isAttachedToWidget() const
{
	return m_owningWidget != nullptr;
}