#include <QtGui/QApplication>
#include "main_ui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainUI w;
    w.show();
    
    return a.exec();
}
