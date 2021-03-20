#ifndef PROTOBUF_HANDLER_HPP
#define PROTOBUF_HANDLER_HPP

#include "IFormatHandler.hpp"

class ProtobufHandler : public IFormatHandler {
public:
    QString formatName() const override { return "Protobuf"; }
    QStringList supportedExtensions() const override { return {"rmt", "rmb"}; }
    QString fileFilter() const override {
        return "Protobuf Register Map (*.rmt *.rmb);;Text Format (*.rmt);;Binary Format (*.rmb)";
    }

    FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
    FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
};

#endif // PROTOBUF_HANDLER_HPP
