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

    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // 1. SSBO (データの流し込み)
    if (m_ssbo) glDeleteBuffers(1, &m_ssbo);
    glGenBuffers(1, &m_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, num * sizeof(Splat::GaussianSplat), splats.data(), GL_STATIC_DRAW);
    // Render時に呼んでも良いですが、ここで初期バインド位置を指定
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);

    // 2. EBO (VAOに紐付け)
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    // 最初は空で確保。Render時にソート済みデータを流す
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

    // 重要: VAOをアンバインドする前にEBOをバインドしておくことで、
    // VAOがこのEBOを「自分のインデックスバッファ」として記憶します。
    glBindVertexArray(0);
}

void Renderer::Render(int num, const float* view, const float* proj, int w, int h, const unsigned int* indices) {
    if (num <= 0 || !indices || m_program == 0) return;

    // --- 1. ブレンド設定の最適化 ---
    glEnable(GL_BLEND);
    // 3DGS公式に近い Premultiplied Alpha 設定
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(m_program);

    // --- 2. 不足している Uniform の転送 ---
    glUniformMatrix4fv(glGetUniformLocation(m_program, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "projection"), 1, GL_FALSE, proj);
    // これを忘れるとジオメトリシェーダーでサイズが0になります
    glUniform2f(glGetUniformLocation(m_program, "screenSize"), (float)w, (float)h);

    // --- 3. バッファの更新と描画 ---
    // SSBOを明示的にバインド (念のため)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);

    // ソート済みインデックスを転送
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, num * sizeof(unsigned int), indices);

    glBindVertexArray(m_vao);
    // インデックスバッファを使用して描画
    glDrawElements(GL_POINTS, num, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}