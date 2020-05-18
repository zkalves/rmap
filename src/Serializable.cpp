#include "Serializable.hpp"
class Serializable
{
public:
    virtual void serialize( QVariantMap& data, SerializationContext* context ) const = 0;
    virtual void deserialize( const QVariantMap& data, SerializationContext* context ) = 0;
};
