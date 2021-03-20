#ifndef CMSIS_SVD_HANDLER_HPP
#define CMSIS_SVD_HANDLER_HPP

#include "IFormatHandler.hpp"

class CmsisSvdHandler : public IFormatHandler {
public:
    CmsisSvdHandler() = default;
    ~CmsisSvdHandler() override = default;

    QString formatName() const override { return "ARM CMSIS-SVD"; }
    QStringList supportedExtensions() const override { return {"svd"}; }
    QString fileFilter() const override { return "ARM CMSIS-SVD (*.svd)"; }

    FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
    FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
};

#endif // CMSIS_SVD_HANDLER_HPP
