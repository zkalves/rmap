#include "SerializationContext.hpp"

void SerializationContext::append_record( QObject* object, QVariantMap data )
{
    Record record;
    record.m_data=data;
    record.m_object=object;
    m_records.append( record );
}
