#include <QApplication>
#include "openglwidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    OpenGLWidget widget;
    widget.resize(800, 600);
    widget.setWindowTitle("3DCube_DrawArrays- Qt OpenGL");
    widget.show();

    return app.exec();
}
