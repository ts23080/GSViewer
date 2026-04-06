#pragma once
#include <glad/glad.h>
#include <string>
#include "Loading.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Init(const std::string& v, const std::string& g, const std::string& f);
    void SetupBuffers(const Loading& loader);

    // ★ここが重要：引数を 5個 (int, float*, float*, int, int) に定義
    void Render(int num, float* view, float* proj, int w, int h);

private:
    GLuint m_program, m_ssbo, m_vao;
    GLuint Compile(GLenum type, const std::string& path);
};