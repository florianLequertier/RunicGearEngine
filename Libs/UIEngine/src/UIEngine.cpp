
#include <memory>

class IUIDrawer
{

};

class IWidgetDrawer
{

};

class UIEngine
{
private:
    std::unique_ptr<IUIDrawer> m_drawer;
};