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
    T* instance = nullptr;

    //if(name == "T")
    //    instance = new T();

    //if(name == "two")
    //    instance = new DerivedClassTwo();

    instance = new T();
    if(instance != nullptr)
        return instance;
    else
        return nullptr;
}
#endif // OBJECTFACTORY_HPP
