#ifndef PROTOBUFLOGCOLLECTOR_HPP
#define PROTOBUFLOGCOLLECTOR_HPP
#include <google/protobuf/io/tokenizer.h>
#include <string>
class ProtobufLogCollector : public google::protobuf::io::ErrorCollector {
    public:
        ProtobufLogCollector() {}
        ~ProtobufLogCollector() {}

        std::string m_str = "";

        void AddError(int line, int column, const std::string& message) {
            m_str = m_str + "ERROR ("   + std::to_string(line+1) + "," + std::to_string(column+1) + "):" + message + "\n";
        }
        void AddWarning(int line, int column, const std::string& message) {
            m_str = m_str + "WARNING (" + std::to_string(line+1) + "," + std::to_string(column+1) + "):" + message + "\n";
        }

        std::string get_string() { return(m_str);}
};
#endif
