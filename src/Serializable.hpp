#ifndef SERIALIZABLE_HPP
#define SERIALIZABLE_HPP

#include <SerializationContext.hpp>
class Serializable
{
public:
    virtual void serialize( QVariantMap& data, SerializationContext* context ) const = 0;
    virtual void deserialize( const QVariantMap& data, SerializationContext* context ) = 0;
};

#endif
