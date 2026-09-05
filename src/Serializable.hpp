/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP

class SerializationContext;
class Serializable
{
public:
    virtual ~Serializable() = default;
    virtual void serialize( QVariantMap& data, SerializationContext* context ) const = 0;
    virtual void deserialize( const QVariantMap& data, SerializationContext* context ) = 0;
};

#endif
