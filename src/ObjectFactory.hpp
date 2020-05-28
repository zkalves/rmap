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
    static QObject* createObject(QByteArray type);
};

#endif // OBJECTFACTORY_HPP
