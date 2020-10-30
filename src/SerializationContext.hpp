#ifndef SERIALIZATIONCONTEXT_HPP
#define SERIALIZATIONCONTEXT_HPP

#include <QVariant>
#include <Serializable.hpp>
#include <ObjectFactory.hpp>
#include <QDebug>
#include "rmap.pb.h"

class SerializationContext
{
public:
    template<typename T>
    QVariant serialize( T* ptr );

    template<typename T>
    T* deserialize( const QVariant& handle );

    friend QDebug               operator <<( QDebug  stream, const SerializationContext& context );
    friend QDataStream&         operator <<( QDataStream& stream, const SerializationContext& context );
    friend QDataStream&         operator >>( QDataStream& stream, SerializationContext& context );
    friend protormap::RegModel& operator <<( protormap::RegModel& reg_model, const SerializationContext& context );
    friend protormap::RegModel& operator >>( protormap::RegModel& reg_model, SerializationContext& context );

private:
    struct Record
    {
        QObject* m_object;
        QByteArray m_type;
        QVariantMap m_data;
    };

private:
    QList<Record> m_records;
    QHash<QObject*, int> m_map;
};

template<typename T>
QVariant SerializationContext::serialize( T* ptr )
{
    if ( ptr == NULL )
        return QVariant();

    QObject* object = static_cast<QObject*>( ptr );

    QHash<QObject*, int>::iterator it = m_map.find( object );

    if ( it != m_map.end() )
        return QVariant( it.value() );

    int index = m_records.count();

    Record record;
    record.m_object = object;
    record.m_type = object->metaObject()->className();
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
        return NULL;

    int index = handle.toInt();

    Record& record = m_records[ index ];

    if ( record.m_object != NULL )
        return static_cast<T*>( record.m_object );

    QObject* object = ObjectFactory::createObject( record.m_type );

    record.m_object = object;

    m_map.insert( object, index );

    T* ptr = static_cast<T*>( object );
    Serializable* serializable = static_cast<Serializable*>( ptr );

    serializable->deserialize( record.m_data, this );

    return ptr;
}
#endif
