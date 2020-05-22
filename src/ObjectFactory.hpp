#ifndef OBJECTFACTORY_HPP
#define OBJECTFACTORY_HPP

class RegMapTreeItem;
#include "RegMapTreeItem.hpp"
#include <memory>
#include <string>
#include <QByteArray>

class ObjectFactory
{
public:
    static std::shared_ptr<RegMapTreeItem> createObject(QByteArray type);
};

#endif // OBJECTFACTORY_HPP
