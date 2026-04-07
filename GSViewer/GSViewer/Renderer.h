#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>
#include <iostream>
#include "Loading.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    // シェーダーの初期化
    bool Init(const std::string& vPath, const std::string& gPath, const std::string& fPath);

    // バッファの構築
    void SetupBuffers(const Loading& loader);

    // 描画実行
    void Render(int num, const float* view, const float* proj, int w, int h);

private:
    GLuint m_program;
    GLuint m_vao;
    GLuint m_vbo;

    // 内部用：シェーダーコンパイル
    GLuint Compile(GLenum type, const std::string& path);
};