#include "ObjectFactory.hpp"

template<typename T>
std::shared_ptr<T> ObjectFactory<T>::createObject(QByteArray type)
{
    T * instance = nullptr;

    (void) type;

    //if(name == "T")
    //    instance = new T();

    //if(name == "two")
    //    instance = new DerivedClassTwo();

    instance = new T();
    if(instance != nullptr)
        return std::shared_ptr<T>(instance);
    else
        return nullptr;
}
