#include "Renderer.h"
#include "Loading.h"
#include <iostream>
#include <vector>

Renderer::Renderer() : m_vao(0), m_vbo(0), m_program(0) {}

Renderer::~Renderer() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_program) glDeleteProgram(m_program);
}

void Renderer::Setup(const std::vector<Loading::GaussianSplat>& splats, GLuint program) {
    m_program = program;

    if (splats.empty()) return;

    // 1. VAOとVBOの生成
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // 2. データの転送
    glBufferData(GL_ARRAY_BUFFER, splats.size() * sizeof(Loading::GaussianSplat), splats.data(), GL_STATIC_DRAW);

    // 3. 頂点属性の設定 (Vertex Attribute Pointers)
    // 1頂点の合計サイズ
    GLsizei stride = sizeof(Loading::GaussianSplat);

    // Location 0: Position (px, py, pz)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Loading::GaussianSplat, px));

    // Location 1: SH Base Color (r, g, b)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Loading::GaussianSplat, r));

    // ※ Location 2 (sh_rest[45]) はシェーダーで使わないためスキップ

    // Location 3: Opacity
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Loading::GaussianSplat, opacity));

    // Location 4: Scale (sx, sy, sz)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Loading::GaussianSplat, sx));

    // Location 5: Rotation (rx, ry, rz, rw)
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Loading::GaussianSplat, rx));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::cout << "Renderer: VAO/VBO Setup complete with " << splats.size() << " splats." << std::endl;
}

void Renderer::Render(int numSplats, const float* view, const float* proj, int screenW, int screenH) {
    if (numSplats <= 0 || m_vao == 0 || m_program == 0) return;

    // アルファブレンディングの設定 (重要: これがないとガウスの重なりが汚くなります)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 3DGSは基本的に順序依存のレンダリングですが、
    // ソート未実装の場合は一旦デプスをオフにすると重なりが見えるようになります
    glDisable(GL_DEPTH_TEST);

    glUseProgram(m_program);

    // 行列の転送
    glUniformMatrix4fv(glGetUniformLocation(m_program, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "projection"), 1, GL_FALSE, proj);

    // 必要に応じてスクリーンサイズを渡す（シェーダー側にある場合）
    GLint screenLoc = glGetUniformLocation(m_program, "screenSize");
    if (screenLoc != -1) {
        glUniform2f(screenLoc, (float)screenW, (float)screenH);
    }

    glBindVertexArray(m_vao);

    // Pointsとして描画し、Geometry ShaderでQuadに変換する
    glDrawArrays(GL_POINTS, 0, numSplats);

    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_BLEND);
}