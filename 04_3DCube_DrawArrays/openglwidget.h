#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QTimer>
#include <QVector3D>
#include <QMatrix4x4>  //

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private slots:
    void updateAnimation();

private:
    QOpenGLShaderProgram *program;
    QOpenGLBuffer vbo;
    QOpenGLVertexArrayObject vao;
    QTimer *animationTimer;

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;

    float rotationAngle = 0.0f;

    void setupCubeData();
    void setupShaders();
};

#endif
