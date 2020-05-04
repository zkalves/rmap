#include <QApplication>
//#include <QCommandLineParser>
//#include <QCommandLineOption>

#include "RegMapWindow.hpp"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    QApplication app(argc, argv);
    //QCoreApplication::setOrganizationName("QtProject");
    //QCoreApplication::setApplicationName("Application Example");
    //QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    //QCommandLineParser parser;
    //parser.setApplicationDescription(QCoreApplication::applicationName());
    //parser.addHelpOption();
    //parser.addVersionOption();
    //parser.addPositionalArgument("file", "The file to open.");
    //parser.process(app);

    RegMapWindow mainWin;
    //if (!parser.positionalArguments().isEmpty())
    //    mainWin.loadFile(parser.positionalArguments().first());
    mainWin.show();
    return app.exec();
}
