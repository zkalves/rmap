#ifndef CSV_HANDLER_HPP
#define CSV_HANDLER_HPP

#include "IFormatHandler.hpp"

class CsvHandler : public IFormatHandler {
public:
    QString formatName() const override { return "CSV Spreadsheet"; }
    QStringList supportedExtensions() const override { return {"csv", "tsv"}; }
    QString fileFilter() const override {
        return "CSV Table (*.csv *.tsv)";
    }

    FormatResult read(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
    FormatResult write(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config) override;
};

#endif // CSV_HANDLER_HPP
