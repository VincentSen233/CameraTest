#include "CameraTest.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    CameraTest window;
    
    window.show();
    return app.exec();
}
