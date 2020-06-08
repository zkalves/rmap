#include <google/protobuf/text_format.h>
class ProtobufLogCollector : public google::protobuf::io::ErrorCollector {
    public:
        ProtobufLogCollector() {}
        ~ProtobufLogCollector() {}

        void AddError(int line, int column, const string& message) { std::cout << message << std::endl; }
        void AddWarning(int line, int column, const string& message) { std::cout << message << std::endl; }
};
