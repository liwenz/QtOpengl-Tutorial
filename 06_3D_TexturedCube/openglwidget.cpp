#include "openglwidget.h"
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QFont>
#include <QMessageBox>
#include <QtMath>

// ------------------- Shader Source Code (Embedded) -------------------
// Vertex Shader Source
const char *vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 position; // Vertex position
layout (location = 1) in vec2 texCoord;   // Texture coordinates

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    TexCoord = texCoord;
}
)glsl";

// Fragment Shader Source
const char *fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D textureSampler; // Texture sampler

void main()
{
    // Sample color from the texture
    FragColor = texture(textureSampler, TexCoord);
}
)glsl";
// ------------------- Vertex and Index Data -------------------
// 3D Dice Cube Data (Position (x, y, z) + Texture Coords (u, v))
float cubeVertices[] = {
    // Face 1: Front (+Z) - Corresponds to face '1'
    // Vertex Position (3)  // Texture Coordinates (2)
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,

    // Face 2: Back (-Z) - Corresponds to face '6'
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,

    // Face 3: Top (+Y) - Corresponds to face '5'
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

    // Face 4: Bottom (-Y) - Corresponds to face '2'
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,

    // Face 5: Right (+X) - Corresponds to face '3'
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f, 1.0f,

    // Face 6: Left (-X) - Corresponds to face '4'
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

// 6 faces, 6 indices per face (2 triangles)
unsigned int cubeIndices[] = {
    // Face 1 (+Z) - Indices 0 to 5
    0, 1, 2,
    2, 3, 0,
    // Face 2 (-Z) - Indices 6 to 11
    4, 5, 6,
    6, 7, 4,
    // Face 3 (+Y) - Indices 12 to 17
    8, 9, 10,
    10, 11, 8,
    // Face 4 (-Y) - Indices 18 to 23
    12, 13, 14,
    14, 15, 12,
    // Face 5 (+X) - Indices 24 to 29
    16, 17, 18,
    18, 19, 16,
    // Face 6 (-X) - Indices 30 to 35
    20, 21, 22,
    22, 23, 20
};

// ------------------- Constructor and Destructor -------------------

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
    rotationAngle(0.0f)
{
    // Initialize textures array pointers to nullptr
    for (int i = 0; i < 6; ++i) {
        textures[i] = nullptr;
    }

    // Setup timer for animation
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &OpenGLWidget::updateAnimation);
    animationTimer->start(16); // Approximately 60 FPS
}

OpenGLWidget::~OpenGLWidget()
{
    // Destructor: Ensure resources are released before destroying the OpenGL Context
    makeCurrent();

    if (program) {
        delete program;
        program = nullptr;
    }

    // Delete the 6 texture pointers
    for (int i = 0; i < 6; ++i) {
        if (textures[i]) {
            delete textures[i];
            textures[i] = nullptr;
        }
    }

    vbo.destroy();
    vao.destroy();

    if (ebo != 0) {
        glDeleteBuffers(1, &ebo);
        ebo = 0;
    }

    doneCurrent();
}

// ------------------- GL Core Functions -------------------

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    qDebug() << "OpenGL version: " << (char*)glGetString(GL_VERSION);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    setupShaders();
    setupCubeData();

    loadTextures(); // Load textures
}

void OpenGLWidget::resizeGL(int w, int h)
{
    projection.setToIdentity();
    projection.perspective(45.0f, (float)w / (float)h, 0.1f, 100.0f);
}

void OpenGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program->bind();
    vao.bind();

    // View Matrix (Camera)
    view.setToIdentity();
    view.lookAt(QVector3D(0.0f, 0.0f, 3.0f),
                QVector3D(0.0f, 0.0f, 0.0f),
                QVector3D(0.0f, 1.0f, 0.0f));

    program->setUniformValue("view", view);
    program->setUniformValue("projection", projection);

    // Model Matrix (Rotation Animation)
    model.setToIdentity();
    model.rotate(rotationAngle, 0.0f, 1.0f, 0.0f); // Rotate around Y-axis
    model.rotate(rotationAngle / 2.0f, 1.0f, 0.0f, 0.0f); // Rotate around X-axis
    program->setUniformValue("model", model);

    // Bind EBO (Element Buffer Object)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    // Draw 6 faces, binding the corresponding texture for each face
    for (int i = 0; i < 6; ++i)
    {
        if (textures[i]) {
            glActiveTexture(GL_TEXTURE0);
            textures[i]->bind();
            program->setUniformValue("textureSampler", 0);

            // Draw the i-th face (6 indices per face)
            // Offset i * 6 * sizeof(unsigned int)
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * 6 * sizeof(unsigned int)));

            textures[i]->release();
        }
    }

    vao.release();
    program->release();
}

// ------------------- Shader and Data Setup -------------------

void OpenGLWidget::setupShaders()
{
    program = new QOpenGLShaderProgram(this);

    // Compile using embedded string source code
    if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource))
        qDebug() << "Vertex shader compilation failed:" << program->log();

    if (!program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource))
        qDebug() << "Fragment shader compilation failed:" << program->log();

    if (!program->link())
        qDebug() << "Shader program linking failed:" << program->log();
}

void OpenGLWidget::setupCubeData()
{
    vao.create();
    vao.bind();

    vbo.create();
    vbo.bind();
    vbo.allocate(cubeVertices, sizeof(cubeVertices));

    // Create and bind EBO (Element Buffer Object)
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    // Vertex position (location 0, defined in GLSL)
    program->enableAttributeArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // Texture coordinates (location 1, defined in GLSL)
    program->enableAttributeArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    vao.release();
    vbo.release();
}

// ------------------- Texture Loading Helper Function -------------------

/**
 * @brief Tries to load a single texture from a file path, generates a fallback texture with text if failed.
 */
QOpenGLTexture* OpenGLWidget::loadSingleTextureOrFallback(const QString& filePath, const QString& fallbackText)
{
    QOpenGLTexture* newTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    QImage image;

    // 1. Try to load from file (relies on Qt Resource System)
    if (image.load(filePath)) {
        // Flip vertically to match OpenGL/Qt coordinate systems
        // Use explicit bools to avoid C4305 warning in some Qt versions
        image = image.mirrored(false, true);
        newTexture->setData(image);
        qDebug() << "Successfully loaded texture:" << filePath;
    } else {
        // 2. Failed to load, generate fallback texture
        qWarning() << "Failed to load texture:" << filePath << ". Generating fallback texture.";

        const int size = 256;
        QImage fallbackImage(size, size, QImage::Format_RGBA8888);
        fallbackImage.fill(QColor(240, 240, 240)); // Light gray background

        QPainter painter(&fallbackImage);
        painter.setRenderHint(QPainter::Antialiasing);

        // Draw the number/text
        QFont font("Arial", size / 2, QFont::Bold);
        painter.setFont(font);
        painter.setPen(Qt::red);

        // Draw text centered
        painter.drawText(fallbackImage.rect(), Qt::AlignCenter, fallbackText);

        painter.end();

        // Flip the fallback image vertically
        newTexture->setData(fallbackImage.mirrored(false, true));
    }

    // 3. Configure texture parameters
    newTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    newTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    newTexture->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::Repeat);
    newTexture->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::Repeat);
    newTexture->generateMipMaps();

    return newTexture;
}

void OpenGLWidget::loadTextures()
{
    // Load 6 textures according to the face order in cubeVertices (opposite sides add to 7)
    // Note: These paths rely on your project's Qt resource file (.qrc). If the images are missing, the red fallback numbers will be shown.
    textures[0] = loadSingleTextureOrFallback("textures/dice_face_1.png", "1"); // +Z Face (1)
    textures[1] = loadSingleTextureOrFallback("textures/dice_face_6.png", "6"); // -Z Face (6)
    textures[2] = loadSingleTextureOrFallback("textures/dice_face_5.png", "5"); // +Y Face (5)
    textures[3] = loadSingleTextureOrFallback("textures/dice_face_2.png", "2"); // -Y Face (2)
    textures[4] = loadSingleTextureOrFallback("textures/dice_face_3.png", "3"); // +X Face (3)
    textures[5] = loadSingleTextureOrFallback("textures/dice_face_4.png", "4"); // -X Face (4)
}

// ------------------- Animation Slot Function -------------------

void OpenGLWidget::updateAnimation()
{
    rotationAngle += 1.0f;
    if (rotationAngle > 360.0f) {
        rotationAngle -= 360.0f;
    }

    update();
}
