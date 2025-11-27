#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture> // 引入 QOpenGLTexture
#include <QMatrix4x4>   // 引入 QMatrix4x4 (用于 Model, View, Projection 矩阵)
#include <QTimer>       // 引入 QTimer (用于动画)

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

private slots:
    void updateAnimation(); // 动画更新槽函数

private:
    void setupShaders();
    void setupCubeData();
    /**
     * @brief 尝试从文件路径加载单个纹理，失败时生成带有文本的回退纹理。
     * @param filePath 纹理文件的相对路径。
     * @param fallbackText 加载失败时显示在纹理上的文本。
     * @return QOpenGLTexture* 指针。
     */
    QOpenGLTexture* loadSingleTextureOrFallback(const QString& filePath, const QString& fallbackText);
    void loadTextures(); // 加载所有 6 个骰子面的纹理

private:
    QOpenGLShaderProgram *program = nullptr;
    QOpenGLBuffer vbo;
    QOpenGLVertexArrayObject vao;
    unsigned int ebo = 0; // 保持 EBO 为原始 OpenGL ID

    QOpenGLTexture *textures[6]; // 6 个纹理指针数组

    QMatrix4x4 view;
    QMatrix4x4 projection;
    QMatrix4x4 model;
    float rotationAngle;

    QTimer *animationTimer;
};

#endif // OPENGLWIDGET_H
