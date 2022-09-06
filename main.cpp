#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.setWindowTitle("Battery Analyzer");
    w.setWindowIcon(QIcon(":/icons/obrazy/ikona.ico"));

    return a.exec();
}
