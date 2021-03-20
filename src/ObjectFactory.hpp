#ifndef OBJECTFACTORY_HPP
#define OBJECTFACTORY_HPP

#include <memory>
#include <string>
#include <QByteArray>
#include <QObject>


class ObjectFactory
{
public:
    // Return shared pointer instead?
    template<typename T>
    static T* createObject();
};

template<typename T>
T* ObjectFactory::createObject(void)
{
    return new T();
}
#endif // OBJECTFACTORY_HPP
