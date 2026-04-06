#include "Renderer.h"
#include <fstream>
#include <sstream>
#include <iostream>

Renderer::Renderer() : m_program(0), m_ssbo(0), m_vao(0) {}

Renderer::~Renderer() {
    if (m_program) glDeleteProgram(m_program);
    if (m_ssbo) glDeleteBuffers(1, &m_ssbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
}

void Renderer::SetupBuffers(const Loading& loader) {
    if (m_ssbo) glDeleteBuffers(1, &m_ssbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);

    glCreateBuffers(1, &m_ssbo);
    glNamedBufferData(m_ssbo,
        loader.GetSplats().size() * sizeof(Loading::GaussianSplat),
        loader.GetSplats().data(),
        GL_STATIC_DRAW);

    glCreateVertexArrays(1, &m_vao);
}

void Renderer::Render(int num, float* view, float* proj, int w, int h) {
    if (num <= 0 || m_program == 0) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(m_program);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "projection"), 1, GL_FALSE, proj);
    glUniform2f(glGetUniformLocation(m_program, "screenSize"), (float)w, (float)h);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, num);

    glBindVertexArray(0);
    glUseProgram(0);
}

bool Renderer::Init(const std::string& vPath, const std::string& gPath, const std::string& fPath) {
    GLuint vShader = Compile(GL_VERTEX_SHADER, vPath);
    GLuint gShader = Compile(GL_GEOMETRY_SHADER, gPath);
    GLuint fShader = Compile(GL_FRAGMENT_SHADER, fPath);

    if (!vShader || !gShader || !fShader) return false;

    m_program = glCreateProgram();
    glAttachShader(m_program, vShader);
    glAttachShader(m_program, gShader);
    glAttachShader(m_program, fShader);
    glLinkProgram(m_program);

    GLint success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) { return false; }

    glDeleteShader(vShader);
    glDeleteShader(gShader);
    glDeleteShader(fShader);
    return true;
}

GLuint Renderer::Compile(GLenum type, const std::string& path) {
    std::ifstream file(path);
    if (!file) return 0;
    std::stringstream ss;
    ss << file.rdbuf();
    std::string src = ss.str();
    const char* csrc = src.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &csrc, NULL);
    glCompileShader(shader);
    return shader;
}