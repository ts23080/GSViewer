#pragma once
#include <glad/glad.h>
#include <vector>
#include "Loading.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    void Setup(const std::vector<Splat::GaussianSplat>& splats, GLuint program);
    void Render(int num, const float* view, const float* proj, int w, int h, const unsigned int* indices);

private:
    GLuint m_vao;
    GLuint m_ssbo;   // 頂点データ全体を保持
    GLuint m_ebo;    // ソート済みインデックスを保持
    GLuint m_program;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
};