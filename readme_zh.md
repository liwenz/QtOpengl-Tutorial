Qt Modern OpenGL 入门：从彩色三角形到纹理贴图 (核心模式)
本仓库提供了一个分阶段的指南，用于搭建 Qt 项目并使用现代 OpenGL (Core Profile) 绘制图形。教程涵盖了 VAO、VBO、EBO、着色器 和纹理贴图等核心概念。
项目结构 (三阶段)
为了清晰和易于学习，教程被划分为三个独立、可运行的 CMake 项目目录，每个阶段都侧重于一个核心概念：
01_ColoredTriangle: 专注于绝对基础：VAO、VBO、着色器，以及使用 glDrawArrays 绘制。
02_IndexedQuad: 引入 EBO (Element Buffer Object)，使用 glDrawElements 高效绘制彩色四边形。
03_TexturedQuad: 纹理贴图。在四边形的基础上，添加纹理坐标，使用 QOpenGLTexture 加载图片并进行贴图。
阶段一：彩色三角形 (01_ColoredTriangle)
核心关注点: VAO, VBO, 着色器, glDrawArrays。
此阶段的顶点数据包含位置 (vec3) 和颜色 (vec3)。
关键代码片段 (顶点属性设置)
// 步长: 6 * sizeof(float) (3 Pos + 3 Color)
// 位置属性 (loc 0): 偏移量 0
program->setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);

// 颜色属性 (loc 1): 偏移量 3 * sizeof(float)
program->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 3, stride);


阶段二：索引四边形 (02_IndexedQuad)
核心关注点: EBO, glDrawElements，实现高效绘制。
此阶段使用索引数组 (indices) 来定义构成四边形的两个三角形，避免了重复存储顶点数据。
关于 EBO 实现方式的说明（重要）：
为确保最大的兼容性和可靠性，尤其是在 Qt 环境中，EBO 是使用原始 OpenGL ID (unsigned int ebo = 0;) 实现的，而不是使用 QOpenGLBuffer 包装类。
在某些 Qt/驱动配置下，用于 GL_ELEMENT_ARRAY_BUFFER 的 QOpenGLBuffer 包装器可能无法正确维护 VAO 内部的状态捕获，导致显示错误（即四边形不被绘制）。显式使用原始 ID 和 glGenBuffers/glBindBuffer 可以可靠地解决这个状态管理问题。
关键代码片段 (绘制命令)
// 绘制 6 个索引 (2个三角形 = 1个四边形)
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 


阶段三：纹理贴图四边形 (03_TexturedQuad)
核心关注点: 纹理贴图, QOpenGLTexture, UV 坐标。
这是最进阶的阶段。此目录下的 openglwidget.cpp 文件已移除颜色属性，只保留了位置和纹理坐标 (UV)。
1. 优化后的顶点数据结构
顶点数据结构被简化为位置 (3个 float) 和纹理坐标 (2个 float)。总步长为 5 个 float。
// 优化后的顶点数据结构: 每顶点 5 个 float (位置/纹理坐标)
float vertices[] = {
    // Position (Loc 0)     // Texture Coords (Loc 2)
    -0.5f,  0.5f, 0.0f,      0.0f, 1.0f, // 左上
    // ... 其他顶点 ...
};


2. 更新顶点属性设置
属性配置反映了新的 5 个 float 步长：
// 步长: 5 * sizeof(float) (3 Pos + 2 TexCoord)
GLsizei stride = 5 * sizeof(float); 

// 位置属性 (location = 0)
program->setAttributeBuffer(0, GL_FLOAT, 0, 3, stride);

// 纹理坐标属性 (location = 2)
// 偏移量: 3 * sizeof(float)
program->setAttributeBuffer(2, GL_FLOAT, 3 * sizeof(float), 2, stride); 


3. 运行要求
请确保将名为 container.jpg 的纹理图片放置在编译后的可执行文件所在的目录中。如果找不到图片，程序将使用棋盘格图案作为回退纹理。
