Qt Modern OpenGL Tutorial: Drawing Colored and Textured Shapes (Core Profile)
This repository provides a step-by-step guide to setting up a Qt project with modern OpenGL (Core Profile) and drawing shapes using VAO, VBO, EBO, and Shaders.
中文文档 (Chinese)
Project Structure (Three Stages)
The tutorial is divided into three separate, runnable projects, each demonstrating a key concept:
01_ColoredTriangle: Focuses on the absolute basics: VAO, VBO, Shaders, and drawing using glDrawArrays.
02_IndexedQuad: Introduces EBO (Element Buffer Object) to efficiently draw a colored quad using glDrawElements.
03_TexturedQuad: Extends the quad example by adding texture coordinates and loading an image file (QOpenGLTexture).
Stage 1: Colored Triangle (01_ColoredTriangle)
Focus: VAO, VBO, Shaders, glDrawArrays.
The vertex data for this stage includes position (vec3) and color (vec3).
Key Code Snippet (Vertex Attributes)
// Stride: 6 * sizeof(float) (3 Pos + 3 Color)
// Pos (loc 0): offset 0
program->setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);

// Color (loc 1): offset 3 * sizeof(float)
program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, stride);


Stage 2: Indexed Quad (02_IndexedQuad)
Focus: EBO, glDrawElements for efficient drawing.
This stage reuses the Position + Color vertex structure from Stage 1 but uses indices to define the two triangles that form the quad, eliminating redundant vertex storage.
Important Note on EBO Implementation:
For maximum compatibility and reliability, especially in Qt environments, the EBO is implemented using a raw OpenGL ID (unsigned int ebo = 0;) instead of the QOpenGLBuffer wrapper class.
In some Qt/driver configurations, the QOpenGLBuffer wrapper for GL_ELEMENT_ARRAY_BUFFER may not correctly maintain the state capture within the VAO, leading to display errors (where the quad is not drawn). Using the raw ID and glGenBuffers/glBindBuffer explicitly solves this state management issue.
Key Code Snippet (Drawing Command)
// 绘制 6 个索引 (2个三角形 = 1个四边形)
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 


Stage 3: Textured Quad (03_TexturedQuad)
Focus: Texture Mapping, QOpenGLTexture, UV Coordinates.
This is the most advanced stage. The openglwidget.cpp file in this directory has been optimized to remove the color attribute, only keeping Position and Texture Coordinates (UV).
1. Optimized Vertex Data Structure
The vertex data is simplified to Position (3 floats) and Texture Coords (2 floats). The total stride is now 5 floats.
// Optimized Vertex Data Structure: 5 floats per vertex (Position/Texture Coords)
float vertices[] = {
    // Position (Loc 0)     // Texture Coords (Loc 2)
    -0.5f,  0.5f, 0.0f,      0.0f, 1.0f, // Top-Left
    // ... other vertices ...
};


2. Updated Vertex Attributes
The attribute configuration reflects the new 5-float stride:
// Stride: 5 * sizeof(float) (3 Pos + 2 TexCoord)
GLsizei stride = 5 * sizeof(float); 

// Position (location = 0)
program->setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);

// Texture Coords (location = 2)
// Offset: 3 * sizeof(float)
program->setAttributeBuffer(2, GL_FLOAT, 3 * sizeof(float), 2, stride); 


3. Requirements
Place an image named container.jpg in the same directory as your compiled executable for the texture loading to succeed. If the image is not found, a checkerboard pattern will be used as a fallback.
