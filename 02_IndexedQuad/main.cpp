#include <QApplication>
#include "openglwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    OpenGLWidget widget;
    widget.resize(800, 600);
    widget.setWindowTitle("2D Color Triangle - Qt OpenGL");
    widget.show();

    return app.exec();
}
