#include "ProtobufLogCollector.hpp"

void ProtobufLogCollector::AddError(int line, int column, const std::string& message) {
    m_str += "ERROR (" + std::to_string(line + 1) + "," + std::to_string(column + 1) + "):" + message + "\n";
}

void ProtobufLogCollector::AddWarning(int line, int column, const std::string& message) {
    m_str += "WARNING (" + std::to_string(line + 1) + "," + std::to_string(column + 1) + "):" + message + "\n";
}

