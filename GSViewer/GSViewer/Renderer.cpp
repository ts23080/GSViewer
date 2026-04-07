#include "Renderer.h"
#include <fstream>
#include <sstream>

Renderer::Renderer() : m_program(0), m_vao(0), m_vbo(0) {}

Renderer::~Renderer() {
    if (m_program) glDeleteProgram(m_program);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
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
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_program, 512, NULL, infoLog);
        std::cerr << "Shader Link Error:\n" << infoLog << std::endl;
        return false;
    }

    glDeleteShader(vShader);
    glDeleteShader(gShader);
    glDeleteShader(fShader);
    return true;
}

void Renderer::SetupBuffers(const Loading& loader) {
    if (loader.GetSplats().empty()) return;

    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, loader.GetSplats().size() * sizeof(Loading::GaussianSplat), loader.GetSplats().data(), GL_STATIC_DRAW);

    GLsizei stride = sizeof(Loading::GaussianSplat);

    // Location 0: Pos, 1: Color, 5: Opacity, 6: Scale, 7: Rot
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Loading::GaussianSplat, px));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Loading::GaussianSplat, r));

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Loading::GaussianSplat, opacity));

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Loading::GaussianSplat, sx));

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Loading::GaussianSplat, rx));

    glBindVertexArray(0);
}

void Renderer::Render(int num, const float* view, const float* proj, int w, int h) {
    if (num <= 0 || m_program == 0) return;

    glUseProgram(m_program);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "projection"), 1, GL_FALSE, proj);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, num);
    glBindVertexArray(0);
}

GLuint Renderer::Compile(GLenum type, const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return 0;
    std::stringstream ss;
    ss << file.rdbuf();
    std::string srcStr = ss.str();
    const char* src = srcStr.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Compile Error (" << path << "):\n" << infoLog << std::endl;
        return 0;
    }
    return shader;
}