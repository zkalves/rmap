#ifndef SERIALIZATIONCONTEXT_HPP
#define SERIALIZATIONCONTEXT_HPP

#include <QVariant>
#include <Serializable.hpp>
#include <ObjectFactory.hpp>

class SerializationContext
{
public:
    template<typename T>
    QVariant serialize( T* ptr );

    template<typename T>
    T* deserialize( const QVariant& handle );

    friend QDataStream& operator <<( QDataStream& stream, const SerializationContext& context );
    friend QDataStream& operator >>( QDataStream& stream, SerializationContext& context );

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

#endif
