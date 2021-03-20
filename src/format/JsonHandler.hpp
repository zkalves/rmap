#ifndef JSON_HANDLER_HPP
#define JSON_HANDLER_HPP

#include "IFormatHandler.hpp"

class JsonHandler : public IFormatHandler {
public:
    QString formatName() const override { return "JSON Schema"; }
    QStringList supportedExtensions() const override { return {"json"}; }
    QString fileFilter() const override {
        return "JSON Register Map (*.json)";
    }

    FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
    FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
};

#endif // JSON_HANDLER_HPP
