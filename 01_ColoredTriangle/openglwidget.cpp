#include "openglwidget.h"
#include <QDebug>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

// ==========================================================
// 1. 顶点数据 (Triangle) - 3个顶点，包含位置和颜色
// ==========================================================
float vertices[] = {
    // Position (Location 0)     // Color (Location 1)
    0.0f,  0.5f, 0.0f,           1.0f, 0.0f, 0.0f,  // Vertex 0: Top, Red
    -0.5f, -0.5f, 0.0f,          0.0f, 1.0f, 0.0f,  // Vertex 1: Bottom-Left, Green
    0.5f, -0.5f, 0.0f,          0.0f, 0.0f, 1.0f   // Vertex 2: Bottom-Right, Blue
};

// 注意：使用 glDrawArrays 方式时，不再需要索引数组 (EBO)

// === 2. 顶点着色器 ===
const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "    ourColor = aColor;\n"
    "}\0";

// === 3. 片段着色器 ===
const char *fragmentShaderSource =
    "#version 330 core\n"
    "in vec3 ourColor;\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(ourColor, 1.0f);\n"
    "}\n\0";

// ==========================================================
// 4. 核心类实现
// ==========================================================
OpenGLWidget::OpenGLWidget(QWidget *parent) : QOpenGLWidget(parent) {}

OpenGLWidget::~OpenGLWidget() {
    makeCurrent();
    vao.destroy();
    vbo.destroy();
    // 析构函数中移除了 EBO 的清理
    delete program;
    doneCurrent();
}

void OpenGLWidget::initializeGL()
{
    qDebug() << "Triangle initialization started.";
    initializeOpenGLFunctions();
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    program = new QOpenGLShaderProgram(this);

    if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource) ||
        !program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource) ||
        !program->link()) {
        qDebug() << "Shader linking failed:" << program->log();
        return;
    }
    qDebug() << "Shaders linked successfully.";

    // 1. 设置 VAO
    vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    // 2. 设置 VBO (仅 VBO，不使用 EBO)
    vbo.create();
    vbo.bind();
    vbo.allocate(vertices, sizeof(vertices));

    // 3. 移除了 EBO 的设置

    // 4. 设置顶点属性
    GLsizei stride = 6 * sizeof(float); // 步长: 3 Pos + 3 Color
    program->bind();

    // 位置属性 (location 0)
    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);

    // 颜色属性 (location 1)
    program->enableAttributeArray(1);
    program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, stride);

    program->release();
    vbo.release();
    qDebug() << "Triangle initialization finished.";
}

void OpenGLWidget::paintGL()
{
    qDebug() << "Drawing triangle...";
    glClear(GL_COLOR_BUFFER_BIT);

    program->bind();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    // 使用 glDrawArrays 绘制：从顶点 0 开始，绘制 3 个顶点 (1个三角形)
    glDrawArrays(GL_TRIANGLES, 0, 3);

    program->release();
    qDebug() << "Triangle drawn successfully.";
}

void OpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}
