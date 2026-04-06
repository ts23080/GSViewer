#include "Renderer.h"
#include <fstream>
#include <sstream>
#include <iostream>

Renderer::Renderer() : m_program(0), m_ssbo(0), m_dummyVao(0) {}

Renderer::~Renderer() {
    glDeleteProgram(m_program);
    glDeleteBuffers(1, &m_ssbo);
    glDeleteVertexArrays(1, &m_dummyVao);
}

void Renderer::SetupBuffers(const Loading& loader) {
    // --- SSBO の作成 ---
    glGenBuffers(1, &m_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);

    // Loadingクラスが保持している全データをGPUへ転送
    glBufferData(GL_SHADER_STORAGE_BUFFER,
        loader.GetSplats().size() * sizeof(Loading::GaussianSplat),
        loader.GetSplats().data(),
        GL_STATIC_DRAW);

    // binding = 0番に接続（シェーダー側と合わせる）
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // --- VAO の作成 (中身は空でOK) ---
    glGenVertexArrays(1, &m_dummyVao);
}

void Renderer::Render(int numSplats) {
    glUseProgram(m_program);

    // SSBOをバインド（念のため毎回実行）
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);

    glBindVertexArray(m_dummyVao);
    // 頂点データはSSBOにあるので、glDrawArraysは「個数」だけ指定して呼ぶ
    glDrawArrays(GL_POINTS, 0, numSplats);

    glBindVertexArray(0);
    glUseProgram(0);
}

// --- シェーダー読み込み・コンパイル処理 (略式) ---
bool Renderer::InitShaders(const std::string& vPath, const std::string& gPath, const std::string& fPath) {
    GLuint vShader = CompileShader(GL_VERTEX_SHADER, vPath);
    GLuint gShader = CompileShader(GL_GEOMETRY_SHADER, gPath);
    GLuint fShader = CompileShader(GL_FRAGMENT_SHADER, fPath);

    m_program = glCreateProgram();
    glAttachShader(m_program, vShader);
    glAttachShader(m_program, gShader);
    glAttachShader(m_program, fShader);
    glLinkProgram(m_program);

    // リンク後は個別のシェーダーは破棄して良い
    glDeleteShader(vShader);
    glDeleteShader(gShader);
    glDeleteShader(fShader);

    return true;
}

GLuint Renderer::CompileShader(GLenum type, const std::string& path) {
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    std::string src = ss.str();
    const char* csrc = src.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &csrc, NULL);
    glCompileShader(shader);

    // ※本来はここで glGetShaderiv でエラーチェックすべき
    return shader;
}