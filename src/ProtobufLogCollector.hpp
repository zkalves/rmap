#ifndef PROTOBUFLOGCOLLECTOR_HPP
#define PROTOBUFLOGCOLLECTOR_HPP

#include <google/protobuf/io/tokenizer.h>
#include <string>

class ProtobufLogCollector : public google::protobuf::io::ErrorCollector {
public:
    ProtobufLogCollector() = default;
    ~ProtobufLogCollector() override = default;

    void AddError(int line, int column, const std::string& message) override;
    void AddWarning(int line, int column, const std::string& message) override;

    std::string string() const { return m_str; }
    std::string get_string() const { return m_str; }

private:
    std::string m_str;
};

#endif // PROTOBUFLOGCOLLECTOR_HPP
