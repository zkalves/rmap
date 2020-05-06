#include "rmap.hpp"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);
    const QString version("v0.1.0");
    QApplication app(argc, argv);
    app.setApplicationVersion(version);
    QCommandLineParser parser;
    QCommandLineOption f_opt({"f","file"}, "Register Map file", "file");
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(f_opt);
    parser.process(app);
    QString regmap_file = parser.value("file");

    RegMapWindow * mainWin = new RegMapWindow(regmap_file);
    mainWin->show();
    return app.exec();
}
