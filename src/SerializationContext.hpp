/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#ifndef SERIALIZATIONCONTEXT_HPP
#define SERIALIZATIONCONTEXT_HPP

#include <QVariant>
#include "Serializable.hpp"
#include "ObjectFactory.hpp"
#include <QDebug>
#include "rmap.pb.h"

class SerializationContext
{
public:
    template<typename T>
    QVariant serialize( T* ptr );

    template<typename T>
    T* deserialize( const QVariant& handle );

    template<typename T>
    void append_record( T* object, QVariantMap data );

    void clear() { m_records.clear(); m_map.clear(); }

    friend QDebug               operator <<( QDebug  stream, const SerializationContext& context );
    friend protormap::RegModel& operator <<( protormap::RegModel& reg_model, const SerializationContext& context );
    friend protormap::RegModel& operator >>( protormap::RegModel& reg_model, SerializationContext& context );

private:
    struct Record
    {
        QObject* m_object = nullptr;
        QVariantMap m_data;
    };

private:
    QList<Record> m_records;
    QHash<QObject*, int> m_map;
};

template<typename T>
QVariant SerializationContext::serialize( T* ptr )
{
    if ( ptr == nullptr )
        return QVariant();

    QObject* object = static_cast<QObject*>( ptr );

    QHash<QObject*, int>::iterator it = m_map.find( object );

    if ( it != m_map.end() )
        return QVariant( it.value() );

    int index = static_cast<int>(m_records.size());

    Record record;
    record.m_object = object;
    m_records.append( record );

    m_map.insert( object, index );

    Serializable* serializable = static_cast<Serializable*>( ptr );

    QVariantMap data;
    serializable->serialize( data, this );

    m_records[ index ].m_data = data;

    return QVariant( index );
}

template<typename T>
T* SerializationContext::deserialize( const QVariant& handle )
{
    if ( !handle.isValid() )
        return nullptr;

    int index = handle.toInt();
    if ( index < 0 || index >= static_cast<int>(m_records.size()) )
        return nullptr;

    Record& record = m_records[ index ];

    if ( record.m_object != nullptr )
        return static_cast<T*>( record.m_object );


    T* object = ObjectFactory::createObject<T>( );

    record.m_object = object;

    m_map.insert( object, index );

    T* ptr = static_cast<T*>( object );
    Serializable* serializable = static_cast<Serializable*>( ptr );

    serializable->deserialize( record.m_data, this );

    return ptr;
}

template<typename T>
void SerializationContext::append_record( T* object, QVariantMap data )
{
    Record record;
    record.m_data=data;
    record.m_object=object;
    m_records.append( record );
}

#endif
