#ifndef OBJECTFACTORY_HPP
#define OBJECTFACTORY_HPP

#include <memory>
#include <string>
#include <QByteArray>


template<typename T>
class ObjectFactory
{
public:
    static std::shared_ptr<T> createObject(QByteArray type);
};

#endif // OBJECTFACTORY_HPP
