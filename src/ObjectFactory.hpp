/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

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
