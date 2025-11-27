#include "openglwidget.h"
#include <QDebug>
#include <QTimer>
#include <QVector3D>
#include <QMatrix4x4>

// 36 vertices (12 triangles * 3 vertices each)
static const float vertices[] = {
    // Positions (XYZ)          // Colors (RGB)
    // Front face (2 triangles)
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  // Front bottom left, Red
    0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  // Front bottom right, Green
    0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // Front top right, Blue

    0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // Front top right, Blue
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  // Front top left, Yellow
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  // Front bottom left, Red

    // Back face (2 triangles)
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  // Back bottom right, Cyan
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  // Back bottom left, Magenta
    -0.5f,  0.5f, -0.5f,  1.0f, 0.5f, 0.0f,  // Back top left, Orange

    -0.5f,  0.5f, -0.5f,  1.0f, 0.5f, 0.0f,  // Back top left, Orange
    0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,  // Back top right, Gray
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  // Back bottom right, Cyan

    // Right face (2 triangles)
    0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  // Front bottom right, Green
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  // Back bottom right, Cyan
    0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,  // Back top right, Gray

    0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,  // Back top right, Gray
    0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // Front top right, Blue
    0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  // Front bottom right, Green

    // Left face (2 triangles)
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  // Back bottom left, Magenta
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  // Front bottom left, Red
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  // Front top left, Yellow

    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  // Front top left, Yellow
    -0.5f,  0.5f, -0.5f,  1.0f, 0.5f, 0.0f,  // Back top left, Orange
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  // Back bottom left, Magenta

    // Top face (2 triangles)
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  // Front top left, Yellow
    0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // Front top right, Blue
    0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,  // Back top right, Gray

    0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,  // Back top right, Gray
    -0.5f,  0.5f, -0.5f,  1.0f, 0.5f, 0.0f,  // Back top left, Orange
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  // Front top left, Yellow

    // Bottom face (2 triangles)
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  // Back bottom left, Magenta
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  // Back bottom right, Cyan
    0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  // Front bottom right, Green

    0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  // Front bottom right, Green
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  // Front bottom left, Red
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f   // Back bottom left, Magenta
};

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), program(nullptr), rotationAngle(0.0f)
{
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &OpenGLWidget::updateAnimation);
}

OpenGLWidget::~OpenGLWidget()
{
    makeCurrent();
    vao.destroy();
    vbo.destroy();
    delete program;
    doneCurrent();
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    qDebug() << "Initializing OpenGL cube with glDrawArrays...";
    setupShaders();
    setupCubeData();

    animationTimer->start(16); // ~60 FPS

    qDebug() << "OpenGL cube initialized successfully";
    qDebug() << "Total vertices: 36 (12 triangles)";
}

void OpenGLWidget::setupShaders()
{
    program = new QOpenGLShaderProgram(this);

    // Vertex shader - uses per-vertex colors
    const char* vertexShader =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "out vec3 ourColor;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "    ourColor = aColor;\n"
        "}\n";

    // Fragment shader
    const char* fragmentShader =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(ourColor, 1.0);\n"
        "}\n";

    if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader)) {
        qDebug() << "Vertex shader compilation error:" << program->log();
        return;
    }
    if (!program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader)) {
        qDebug() << "Fragment shader compilation error:" << program->log();
        return;
    }
    if (!program->link()) {
        qDebug() << "Shader program link error:" << program->log();
        return;
    }

    qDebug() << "Shaders compiled and linked successfully";
}

void OpenGLWidget::setupCubeData()
{
    // Bind program before setting attributes
    program->bind();

    vao.create();
    vao.bind();

    // Setup Vertex Buffer Object (VBO)
    vbo.create();
    vbo.bind();
    vbo.allocate(vertices, sizeof(vertices));
    qDebug() << "VBO allocated:" << sizeof(vertices) << "bytes";

    // Set up vertex attributes
    // Position attribute (location = 0)
    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));

    // Color attribute (location = 1)
    program->enableAttributeArray(1);
    program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 6 * sizeof(float));

    vao.release();
    program->release();

    qDebug() << "Cube vertex data setup complete";
}

void OpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    projection.setToIdentity();
    projection.perspective(45.0f, float(w)/float(h), 0.1f, 100.0f);
    qDebug() << "Viewport resized to:" << w << "x" << h;
}

void OpenGLWidget::paintGL()
{
    // Clear buffers with light gray background
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!program || !program->isLinked()) {
        qDebug() << "Shader program not ready, skipping draw call";
        return;
    }

    program->bind();
    vao.bind();

    // Set transformation matrices
    program->setUniformValue("projection", projection);

    // View matrix - camera positioned at (0,0,-3) looking at origin
    view.setToIdentity();
    view.translate(0.0f, 0.0f, -3.0f);
    program->setUniformValue("view", view);

    // Model matrix - apply continuous rotation
    model.setToIdentity();
    model.rotate(rotationAngle, QVector3D(0.5f, 1.0f, 0.0f));
    rotationAngle += 1.0f;
    if (rotationAngle >= 360.0f) {
        rotationAngle = 0.0f;
    }
    program->setUniformValue("model", model);

    // Draw the cube using glDrawArrays
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        qDebug() << "OpenGL draw error:" << error;
    }

    vao.release();
    program->release();
}

void OpenGLWidget::updateAnimation()
{
    update(); // Trigger repaint
}
