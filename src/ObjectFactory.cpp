#include "ObjectFactory.hpp"

shared_ptr<MyBaseClass> ObjectFactory::createObject(string name)
{
    MyBaseClass * instance = nullptr;

    if(name == "RegMapTreeItem")
        instance = new RegMapTreeItem();

    //if(name == "two")
    //    instance = new DerivedClassTwo();

    if(instance != nullptr)
        return std::shared_ptr<RegMapTreeItem>(instance);
    else
        return nullptr;
}
