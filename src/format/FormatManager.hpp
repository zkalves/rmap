#ifndef FORMAT_MANAGER_HPP
#define FORMAT_MANAGER_HPP

#include <QString>
#include <QStringList>
#include <vector>
#include <memory>
#include "IFormatHandler.hpp"

class FormatManager {
public:
    static FormatManager& instance();

    void registerHandler(std::shared_ptr<IFormatHandler> handler);
    void clearHandlers();

    std::shared_ptr<IFormatHandler> handlerForFile(const QString &filepath) const;
    std::shared_ptr<IFormatHandler> handlerByName(const QString &name) const;

    FormatResult loadFile(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config);
    FormatResult saveFile(const QString &filepath, RegMapTreeModel *model, RegConfigWindow *config);

    QString allFilterString() const;
    const std::vector<std::shared_ptr<IFormatHandler>>& handlers() const;

private:
    FormatManager();
    void registerDefaultHandlers();

    std::vector<std::shared_ptr<IFormatHandler>> m_handlers;
};

#endif // FORMAT_MANAGER_HPP
