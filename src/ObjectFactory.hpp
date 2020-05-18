#ifndef OBJECTFACTORY_HPP
#define OBJECTFACTORY_HPP

#include "RegMapTreeItem.hpp"
#include <memory>
#include <string>

class ObjectFactory
{
public:
    static shared_ptr<MyBaseClass> createObject(string name);
};

#endif // OBJECTFACTORY_HPP
