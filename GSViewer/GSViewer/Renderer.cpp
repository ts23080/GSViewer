#include "Renderer.h"
#include <iostream>

Renderer::Renderer() : m_vao(0), m_ssbo(0), m_ebo(0), m_program(0) {}

Renderer::~Renderer() {
    if (m_ssbo) glDeleteBuffers(1, &m_ssbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void Renderer::Setup(const std::vector<Splat::GaussianSplat>& splats, GLuint program) {
    m_program = program;
    int num = static_cast<int>(splats.size());

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // 1. SSBO (頂点データ)
    glGenBuffers(1, &m_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, num * sizeof(Splat::GaussianSplat), splats.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo); // binding = 0

    // 2. EBO (インデックス)
    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
}

void Renderer::Render(int num, const float* view, const float* proj, int w, int h, const unsigned int* indices) {
    if (num <= 0 || !indices) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // ソートするためデプスはオフ

    glUseProgram(m_program);

    // 行列転送
    glUniformMatrix4fv(glGetUniformLocation(m_program, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "projection"), 1, GL_FALSE, proj);

    // インデックスバッファを現在のソート順で更新
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, num * sizeof(unsigned int), indices);

    glBindVertexArray(m_vao);
    // SSBOからEBOの順序で点を描画
    glDrawElements(GL_POINTS, num, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}