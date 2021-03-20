#ifndef SYSTEM_RDL_HANDLER_HPP
#define SYSTEM_RDL_HANDLER_HPP

#include "IFormatHandler.hpp"

class SystemRdlHandler : public IFormatHandler {
public:
    QString formatName() const override { return "SystemRDL"; }
    QStringList supportedExtensions() const override { return {"rdl", "systemrdl"}; }
    QString fileFilter() const override {
        return "SystemRDL (*.rdl *.systemrdl)";
    }

    FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
    FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
};

#endif // SYSTEM_RDL_HANDLER_HPP
