#include "ObjectFactory.hpp"

std::shared_ptr<RegMapTreeItem> ObjectFactory::createObject(QByteArray type)
{
    RegMapTreeItem * instance = nullptr;

    (void) type;

    //if(name == "RegMapTreeItem")
    //    instance = new RegMapTreeItem();

    //if(name == "two")
    //    instance = new DerivedClassTwo();

    instance = new RegMapTreeItem();
    if(instance != nullptr)
        return std::shared_ptr<RegMapTreeItem>(instance);
    else
        return nullptr;
}
