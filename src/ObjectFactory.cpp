#include "ObjectFactory.hpp"

QObject* ObjectFactory::createObject(QByteArray type)
{
    QObject* instance = nullptr;

    (void) type;

    //if(name == "T")
    //    instance = new T();

    //if(name == "two")
    //    instance = new DerivedClassTwo();

    instance = new QObject();
    if(instance != nullptr)
        return instance;
    else
        return nullptr;
}
