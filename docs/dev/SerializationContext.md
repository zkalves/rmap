# SerializationContext and Serialization Framework

### 1. Overview

`SerializationContext.hpp` provides a generic object graph serialization engine that maps complex in-memory C++ pointer graphs (such as parent-child tree nodes in `RegMapTreeItem`) into flat index-referenced records suitable for Google Protocol Buffer text (`.rmt`) and binary (`.rmb`) serialization. Together with `Serializable.hpp`, `ObjectFactory.hpp`, and `ProtobufLogCollector.hpp`, it forms the persistence foundation of **rmap**.

### 2. Project Structure and Dependencies

Declared in `src/SerializationContext.hpp`, `src/Serializable.hpp`, `src/ObjectFactory.hpp`, and `src/ProtobufLogCollector.hpp`.
- Utilized by `RegMapTreeItem` and `ProtobufHandler`.
- Bridges `RegMapTreeModel` and the generated Protocol Buffer classes in `rmap.pb.h`.

Build Requirements:
- Qt 6 modules: `QtCore`
- Protocol Buffers compiler and runtime (`libprotobuf`)

### 3. Core Classes and Interfaces

| Class Name | Header | Description |
|---|---|---|
| `Serializable` | `src/Serializable.hpp` | Abstract interface defining `serialize(QVariantMap&, SerializationContext*)` and `deserialize(...)`. |
| `SerializationContext` | `src/SerializationContext.hpp` | Graph manager tracking object pointers, assigning integer handles, and flattening structures. |
| `ObjectFactory` | `src/ObjectFactory.hpp` | Template factory creating default-constructed instances during deserialization. |
| `ProtobufLogCollector` | `src/ProtobufLogCollector.hpp` | Subclass of `google::protobuf::io::ErrorCollector` capturing parse errors and warnings during protobuf decoding. |

### 4. SerializationContext API

#### template<typename T> QVariant serialize(T* ptr)
Registers pointer `ptr` with the context. If already registered, returns its existing integer handle; otherwise, appends a new record and calls `ptr->serialize()` to capture its state.

#### template<typename T> T* deserialize(const QVariant& handle)
Retrieves the object associated with integer `handle`. If not yet instantiated, uses `ObjectFactory::createObject<T>()` to allocate the object and calls `deserialize()` on it.

#### template<typename T> void append_record(T* object, QVariantMap data)
Appends a raw record during stream decoding.

#### friend protormap::RegModel& operator <<(protormap::RegModel& reg_model, const SerializationContext& context)
Streams flattened context records into a `protormap::RegModel` protobuf message.

#### friend protormap::RegModel& operator >>(protormap::RegModel& reg_model, SerializationContext& context)
Decodes a `protormap::RegModel` protobuf message into context records.

### 5. ProtobufLogCollector API

#### void AddError(int line, int column, const std::string& message) [override]
Captures text format parser error messages with line and column indices.

#### void AddWarning(int line, int column, const std::string& message) [override]
Captures text format parser warnings.

#### std::string get_string() const / std::string string() const
Returns the accumulated error log string.

### 6. Ownership and Lifecycle

`SerializationContext` is a transient helper allocated on the stack during file save and load routines. Objects instantiated by `deserialize()` are transferred to the caller (e.g. `RegMapTreeModel::setRootItem`).

### 7. Thread Safety

`SerializationContext` is **single-threaded**.

### 8. Usage Example

```cpp
#include "SerializationContext.hpp"
#include "RegMapTreeItem.hpp"

void saveTreeToProtobuf(RegMapTreeItem *rootItem, protormap::RegModel &model)
{
    SerializationContext context;
    context.serialize(rootItem);
    model << context;
}

RegMapTreeItem* loadTreeFromProtobuf(protormap::RegModel &model)
{
    SerializationContext context;
    model >> context;
    return context.deserialize<RegMapTreeItem>(QVariant::fromValue<int>(0));
}
```
