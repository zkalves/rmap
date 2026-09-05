/* This file is part of rmap.
 *
 * rmap is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rmap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rmap.  If not, see <https://www.gnu.org/licenses/>.
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
