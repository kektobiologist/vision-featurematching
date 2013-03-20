#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<vector<Shape> >("vector<Shape>");
    qRegisterMetaType<Shape>("Shape");
    qDebug() << "vector ID = " << QMetaType::type("vector<Shape>");
    MainWindow w;
    w.show();
    return a.exec();
}
