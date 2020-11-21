#include "ObjectFactory.hpp"

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
