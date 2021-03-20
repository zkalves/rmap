#ifndef IPXACT_HANDLER_HPP
#define IPXACT_HANDLER_HPP

#include "IFormatHandler.hpp"

class IpxactHandler : public IFormatHandler {
public:
    QString formatName() const override { return "IP-XACT (IEEE 1685)"; }
    QStringList supportedExtensions() const override { return {"xml", "ipxact"}; }
    QString fileFilter() const override {
        return "IP-XACT IEEE 1685 (*.xml *.ipxact)";
    }

    FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
    FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
};

#endif // IPXACT_HANDLER_HPP
