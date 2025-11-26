#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QOpenGLShaderProgram *program = nullptr;
    QOpenGLBuffer vbo;
    QOpenGLVertexArrayObject vao;
    unsigned int ebo = 0; // Raw OpenGL ID for EBO
    bool m_firstPaint; // <--- flag
    void loadTexture(const QString& filePath);
    QOpenGLTexture *texture = nullptr;
};

#endif // OPENGLWIDGET_H
