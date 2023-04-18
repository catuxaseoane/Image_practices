#include <QApplication>
#include "fpimage.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FPImage w;
    w.show();
    
    return a.exec();
}
