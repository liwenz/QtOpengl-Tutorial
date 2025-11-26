#include "openglwidget.h"
#include <QDebug>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

// ==========================================================
// 1. 顶点数据 (Quad) - 4个顶点，包含位置 (XYZ) 和颜色 (RGB)
// ==========================================================
float vertices[] = {
    // Position (Location 0)     // Color (Location 1)
    -0.5f,  0.5f, 0.0f,          1.0f, 0.0f, 0.0f,  // 顶点 0: 左上，红色
    -0.5f, -0.5f, 0.0f,          0.0f, 1.0f, 0.0f,  // 顶点 1: 左下，绿色
    0.5f, -0.5f, 0.0f,          0.0f, 0.0f, 1.0f,  // 顶点 2: 右下，蓝色
    0.5f,  0.5f, 0.0f,          1.0f, 1.0f, 0.0f   // 顶点 3: 右上，黄色
};

// ==========================================================
// 2. 索引数据 (EBO) - 6个索引，通过两个三角形构成四边形
// ==========================================================
unsigned int indices[] = {
    0, 1, 2,  // Triangle 1
    2, 3, 0   // Triangle 2
};

// === 3. 顶点着色器 (传递位置和颜色) ===
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

// === 4. 片段着色器 (接收并使用内插后的颜色) ===
const char *fragmentShaderSource =
    "#version 330 core\n"
    "in vec3 ourColor;\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(ourColor, 1.0f);\n"
    "}\n\0";

// ==========================================================
// 5. 构造函数和析构函数
// ==========================================================
OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
}

OpenGLWidget::~OpenGLWidget()
{
    makeCurrent();
    vao.destroy();
    vbo.destroy();
    // 使用原生 OpenGL API 释放 EBO
    if (ebo != 0) {
        glDeleteBuffers(1, &ebo);
    }
    delete program;
    doneCurrent();
}

// ==========================================================
// 6. 初始化函数
// ==========================================================
void OpenGLWidget::initializeGL()
{
    qDebug() << "Initialization started.";
    initializeOpenGLFunctions();

    // 设置背景色
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    program = new QOpenGLShaderProgram(this);

    // 编译和链接着色器
    if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource) ||
        !program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource) ||
        !program->link())
    {
        qDebug() << "Shader error:" << program->log();
        return;
    }

    qDebug() << "Shaders linked successfully.";

    // 1. 设置 VAO
    vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    // 2. 设置 VBO (顶点数据)
    vbo.create();
    vbo.bind();
    vbo.allocate(vertices, sizeof(vertices));

    // 3. 设置 EBO (索引数据)
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 4. 设置顶点属性
    GLsizei stride = 6 * sizeof(float); // 步长: 3个位置float + 3个颜色float

    program->bind();

    // 位置属性 (location = 0)
    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);

    // 颜色属性 (location = 1)
    program->enableAttributeArray(1);
    // 偏移量: 跳过前面的 3 * sizeof(float) 位置数据
    program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, stride);

    // 释放资源
    program->release();
    vbo.release();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // 释放 EBO 绑定
    qDebug() << "Initialization finished.";
}

// ==========================================================
// 7. 绘制函数 (PaintGL)
// ==========================================================
void OpenGLWidget::paintGL()
{
    // 清除背景
    glClear(GL_COLOR_BUFFER_BIT);

    program->bind();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    // 关键修复: 再次显式绑定 EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    // 绘制 6 个索引 (2个三角形 = 1个四边形)
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // 解绑 EBO

    program->release();
}

void OpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}
