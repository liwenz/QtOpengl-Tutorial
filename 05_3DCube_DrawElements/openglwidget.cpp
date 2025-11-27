#include "openglwidget.h"
#include <QDebug>
#include <QTimer>
#include <QVector3D>
#include <QMatrix4x4>

// 8 unique vertices for the cube
static const float vertices[] = {
    // Positions (XYZ)          // Colors (RGB)
    // Front face vertices
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  // 0: Front bottom left, Red
    0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  // 1: Front bottom right, Green
    0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // 2: Front top right, Blue
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  // 3: Front top left, Yellow

    // Back face vertices
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  // 4: Back bottom left, Magenta
    0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  // 5: Back bottom right, Cyan
    0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f,  // 6: Back top right, Gray
    -0.5f,  0.5f, -0.5f,  1.0f, 0.5f, 0.0f   // 7: Back top left, Orange
};

// Index data - 12 triangles (36 indices)
static const unsigned int indices[] = {
    // Front face
    0, 1, 2,  0, 2, 3,
    // Back face
    5, 4, 7,  5, 7, 6,
    // Right face
    1, 5, 6,  1, 6, 2,
    // Left face
    4, 0, 3,  4, 3, 7,
    // Top face
    3, 2, 6,  3, 6, 7,
    // Bottom face
    4, 5, 1,  4, 1, 0
};

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), program(nullptr), rotationAngle(0.0f), ebo(0)
{
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &OpenGLWidget::updateAnimation);
}

OpenGLWidget::~OpenGLWidget()
{
    makeCurrent();
    vao.destroy();
    vbo.destroy();
    if (ebo != 0) {
        glDeleteBuffers(1, &ebo);
    }
    delete program;
    doneCurrent();
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);

    qDebug() << "Initializing EBO cube...";
    setupShaders();
    setupCubeData();

    animationTimer->start(16); // ~60 FPS

    qDebug() << "EBO Cube initialized successfully";
    qDebug() << "Unique vertices: 8, Total indices: 36";
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

    // Setup Element Buffer Object (EBO) using native OpenGL API
    glGenBuffers(1, &ebo); // Generate EBO ID
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); // Bind EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Allocate data

    qDebug() << "EBO allocated:" << sizeof(indices) << "bytes";

    // Set up vertex attributes
    // Position attribute (location = 0)
    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 6 * sizeof(float));

    // Color attribute (location = 1)
    program->enableAttributeArray(1);
    program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, 6 * sizeof(float));

    vao.release();
    program->release();

    qDebug() << "Cube data setup complete";
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

    // Draw the cube using EBO (glDrawElements)
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

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
