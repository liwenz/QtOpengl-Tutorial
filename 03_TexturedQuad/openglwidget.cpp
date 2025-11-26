#include "openglwidget.h"
#include <QDebug>
#include <QColor>
#include <QImage>
#include <QCoreApplication>
#include <QDir>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QKeyEvent>

// IMPORTANT: Place an image named 'texture.png' in the same directory as your executable.
const QString TARGET_IMAGE_NAME = "texture.png";

// ==========================================================
// 1. Vertex Data (Quad)
// [Position (XYZ), Texture Coordinates (UV)] - Color data removed
// ==========================================================
float vertices[] = {
    // Position (XYZ)    // TexCoord (UV)
    0.5f,  0.5f, 0.0f,   1.0f, 1.0f,  // Top Right
    0.5f, -0.5f, 0.0f,   1.0f, 0.0f,  // Bottom Right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,  // Bottom Left
    -0.5f,  0.5f, 0.0f,   0.0f, 1.0f   // Top Left
};

// ==========================================================
// 2. Index Data (EBO)
// ==========================================================
unsigned int indices[] = {
    0, 1, 3,  // First triangle
    1, 2, 3   // Second triangle
};

// === 3. Vertex Shader === (Color attribute removed)
const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    // Location 1 (Color) is now unused
    "layout (location = 2) in vec2 aTexCoord;\n"

    "out vec2 TexCoord;\n" // Only TexCoord is passed to Fragment Shader

    "void main()\n"
    "{\n"
    "    gl_Position = vec4(aPos, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}\0";

// === 4. Fragment Shader === (Color input removed)
const char *fragmentShaderSource =
    "#version 330 core\n"
    "in vec2 TexCoord;\n"

    "out vec4 FragColor;\n"

    "uniform sampler2D ourTexture;\n"

    "void main()\n"
    "{\n"
    "    FragColor = texture(ourTexture, TexCoord);\n"
    "}\n\0";

// ==========================================================
// 5. Constructor and Destructor
// ==========================================================
OpenGLWidget::OpenGLWidget(QWidget *parent) : QOpenGLWidget(parent), ebo(0), texture(nullptr) {}

OpenGLWidget::~OpenGLWidget() {
    makeCurrent();
    vao.destroy();
    vbo.destroy();
    if (ebo != 0) {
        glDeleteBuffers(1, &ebo);
    }
    delete program;
    if (texture) {
        delete texture;
    }
    doneCurrent();
}

// ==========================================================
// 5.1. Texture Loading Utility
// ==========================================================
void OpenGLWidget::loadTexture(const QString& relativePath)
{
    // --- Construct Absolute Path: Use the executable's directory as the base ---
    QString appDir = QCoreApplication::applicationDirPath();
    QString absolutePath = QDir(appDir).filePath(relativePath);

    // Clean up old texture
    if (texture) {
        delete texture;
        texture = nullptr;
    }

    QImage image;
    // Attempt to load the texture from the absolute path
    bool loaded = image.load(absolutePath);

    if (loaded && !image.isNull()) {
        qDebug() << "Texture loaded successfully from absolute path:" << absolutePath;
        // Mirror Y axis to match OpenGL coordinate system (Qt loads images flipped)
        texture = new QOpenGLTexture(image.mirrored());
    } else {
        qDebug() << "=========================================================================";
        qDebug() << "WARNING: Texture loading failed! Using checkerboard texture as fallback.";
        qDebug() << "         Attempted absolute path: " << absolutePath;
        qDebug() << "=========================================================================";

        // Generate a simple checkerboard texture as fallback
        QImage checkerboardImage(128, 128, QImage::Format_RGB32);
        const int tileSize = 16;
        for (int y = 0; y < 128; ++y) {
            for (int x = 0; x < 128; ++x) {
                bool isDark = ((x / tileSize) % 2 == 0) ^ ((y / tileSize) % 2 == 0);
                QColor color = isDark ? QColor(50, 50, 50) : QColor(200, 200, 200);
                checkerboardImage.setPixel(x, y, color.rgb());
            }
        }

        texture = new QOpenGLTexture(checkerboardImage.mirrored());
    }

    // Configure texture parameters
    if (texture) {
        texture->setMinificationFilter(QOpenGLTexture::Nearest);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
        texture->setWrapMode(QOpenGLTexture::Repeat);
    }
}

// ==========================================================
// 6. Initialization Function (initializeGL)
// ==========================================================
void OpenGLWidget::initializeGL()
{
    qDebug() << "Textured Quad initialization started.";
    initializeOpenGLFunctions();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Dark background

    program = new QOpenGLShaderProgram(this);

    // Load and link shaders
    if (!program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource) ||
        !program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource) ||
        !program->link())
    {
        qDebug() << "Shader error:" << program->log();
        return;
    }

    qDebug() << "Shaders linked successfully.";

    // 1. Setup VAO
    vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    // 2. Setup VBO (Vertex Data)
    vbo.create();
    vbo.bind();
    vbo.allocate(vertices, sizeof(vertices));

    // 3. Setup EBO (Index Data)
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 4. Setup Vertex Attributes
    GLsizei stride = 5 * sizeof(float); // Stride: 3 Pos + 2 TexCoord (Reduced from 8)

    program->bind();

    // Position attribute (location = 0)
    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);

    // Color attribute (location = 1) - REMOVED

    // Texture coordinate attribute (location = 2)
    program->enableAttributeArray(2);
    // Offset is now 3 * sizeof(float), as we skip the 3 Position floats
    program->setAttributeBuffer(2, GL_FLOAT, 3 * sizeof(float), 2, stride);

    // --- Texture Loading ---
    loadTexture(TARGET_IMAGE_NAME);

    // Set uniform sampler to texture unit 0
    program->setUniformValue("ourTexture", 0);

    // Release resources
    program->release();
    vbo.release();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    qDebug() << "OpenGL initialization finished.";
}

// ==========================================================
// 7. Drawing Function (paintGL)
// ==========================================================
void OpenGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    program->bind();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao);

    // Bind texture to unit 0
    if (texture) {
        texture->bind(0);
    }

    // Crucial fix: Explicitly re-bind EBO (for Qt compatibility)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    // Draw 6 indices (2 triangles = 1 quad)
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Unbind resources
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    if (texture) {
        texture->release();
    }
    program->release();
}

void OpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}
