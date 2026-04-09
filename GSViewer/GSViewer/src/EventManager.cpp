#include "EventManager.h"
#include <iostream>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

// --- シェーダーコンパイル用ヘルパー（実装はファイル下部） ---
static GLuint CompileShader(GLenum type, const char* path);
GLuint CreateProgram(const char* vPath, const char* gPath, const char* fPath);

// シングルトンのインスタンス取得
EventManager& EventManager::getInst() {
    static EventManager instance;
    return instance;
}

EventManager::EventManager()
    : m_isL(false), m_isR(false), m_isM(false),
    m_camPos(0, 0, 0), m_camYaw(0), m_camPitch(0), m_camDist(10.0f)
{
    if (m_loader.LoadFromPly("model.ply")) {
        GLuint prog = CreateProgram("shaders/splat.vert", "shaders/splat.geo", "shaders/splat.frag");
        if (prog != 0) {
            m_renderer.Setup(m_loader.GetSplats(), prog);

            // インデックス配列の初期化
            m_indices.resize(m_loader.GetNumSplats());
            std::iota(m_indices.begin(), m_indices.end(), 0);
            std::cout << "EventManager: Initialization Complete." << std::endl;
        }
    }
}

void EventManager::SortSplats() {
    if (m_indices.empty()) return;

    // --- 1. カメラの forward ベクトルを正しく計算 ---
    Eigen::Vector3f forward;
    forward.x() = cos(m_camPitch) * sin(m_camYaw);
    forward.y() = sin(m_camPitch);
    forward.z() = cos(m_camPitch) * cos(m_camYaw);
    forward.normalize();

    Eigen::Vector3f camPos = m_camPos;
    const auto& splats = m_loader.GetSplats();

    // --- 2. depth の事前計算 ---
    std::vector<std::pair<float, unsigned int>> depthIndices(m_indices.size());

    for (size_t i = 0; i < m_indices.size(); ++i) {
        unsigned int idx = m_indices[i];
        const auto& s = splats[idx];

        Eigen::Vector3f relPos(s.px - camPos.x(), s.py - camPos.y(), s.pz - camPos.z());

        // forward 方向への投影（前方が正）
        float d = relPos.dot(forward);

        depthIndices[i] = { d, idx };
    }

    // --- 3. 遠い順（降順）にソート ---
    std::sort(depthIndices.begin(), depthIndices.end(),
        [](const auto& a, const auto& b) {
            return a.first > b.first;  // 遠い → 近い
        }
    );

    // --- 4. インデックス更新 ---
    for (size_t i = 0; i < m_indices.size(); ++i) {
        m_indices[i] = depthIndices[i].second;
    }
}

void EventManager::DrawScene(int w, int h) {
    // 1. 描画前にソートを実行
    SortSplats();

    glViewport(0, 0, w, h);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    EMat4f view = GetViewMatrix();
    EMat4f proj = GetProjectionMatrix((float)w, (float)h);

    // 2. ソート済みインデックス配列(m_indices.data())を渡してレンダリング
    m_renderer.Render(m_loader.GetNumSplats(), view.data(), proj.data(), w, h, m_indices.data());
}

EMat4f EventManager::GetViewMatrix() {
    float x = m_camDist * cos(m_camPitch) * sin(m_camYaw);
    float y = m_camDist * sin(m_camPitch);
    float z = m_camDist * cos(m_camPitch) * cos(m_camYaw);
    EVec3f eye = m_camPos + EVec3f(x, y, z);
    EVec3f center = m_camPos;
    EVec3f up(0, -1, 0);

    EVec3f f = (center - eye).normalized();
    EVec3f s = f.cross(up).normalized();
    EVec3f u = s.cross(f);

    EMat4f res = EMat4f::Identity();
    res(0, 0) = s.x(); res(1, 0) = u.x(); res(2, 0) = -f.x();
    res(0, 1) = s.y(); res(1, 1) = u.y(); res(2, 1) = -f.y();
    res(0, 2) = s.z(); res(1, 2) = u.z(); res(2, 2) = -f.z();
    res(0, 3) = -s.dot(eye);
    res(1, 3) = -u.dot(eye);
    res(2, 3) = f.dot(eye);
    return res;
}

EMat4f EventManager::GetProjectionMatrix(float w, float h) {
    float aspect = w / h;
    float fov = 45.0f * 3.14159265f / 180.0f;
    float zNear = 0.1f, zFar = 1000.0f;
    float f = 1.0f / tan(fov / 2.0f);
    EMat4f res = EMat4f::Zero();
    res(0, 0) = f / aspect; res(1, 1) = f;
    res(2, 2) = (zFar + zNear) / (zNear - zFar);
    res(2, 3) = (2.0f * zFar * zNear) / (zNear - zFar);
    res(3, 2) = -1.0f;
    return res;
}

void EventManager::MouseMove(EVec2i p) {
    if (!m_isL && !m_isR && !m_isM) return;
    float dx = (float)(p.x() - m_prepos.x());
    float dy = (float)(p.y() - m_prepos.y());
    if (m_isL) {
        m_camYaw -= dx * 0.005f; m_camPitch += dy * 0.005f;
        m_camPitch = std::max(-1.5f, std::min(1.5f, m_camPitch));
    }
    else if (m_isR) { m_camPos.y() += dy * 0.01f; m_camPos.x() -= dx * 0.01f; }
    else if (m_isM) { m_camDist += dy * 0.1f; }
    m_prepos = p;
}

void EventManager::MouseWheel(int dx, int dy) { m_camDist -= (float)dy * 0.5f; if (m_camDist < 0.1f) m_camDist = 0.1f; }
void EventManager::LBtnDown(EVec2i p, bool c, bool s) { m_isL = true; m_prepos = p; }
void EventManager::RBtnDown(EVec2i p, bool c, bool s) { m_isR = true; m_prepos = p; }
void EventManager::MBtnDown(EVec2i p, bool c, bool s) { m_isM = true; m_prepos = p; }
void EventManager::LBtnUp(EVec2i p, bool c, bool s) { m_isL = false; }
void EventManager::RBtnUp(EVec2i p, bool c, bool s) { m_isR = false; }
void EventManager::MBtnUp(EVec2i p, bool c, bool s) { m_isM = false; }

// --- シェーダー関連の実装 ---
static GLuint CompileShader(GLenum type, const char* path) {
    std::ifstream f(path);
    if (!f) { std::cerr << "File not found: " << path << std::endl; return 0; }
    std::stringstream ss; ss << f.rdbuf();
    std::string src = ss.str();
    const char* c_src = src.c_str();
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &c_src, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512]; glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader Error [" << path << "]: " << infoLog << std::endl;
        return 0;
    }
    return shader;
}

GLuint CreateProgram(const char* vPath, const char* gPath, const char* fPath) {
    GLuint v = CompileShader(GL_VERTEX_SHADER, vPath);
    GLuint g = CompileShader(GL_GEOMETRY_SHADER, gPath);
    GLuint f = CompileShader(GL_FRAGMENT_SHADER, fPath);
    if (!v || !g || !f) return 0;
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v); glAttachShader(prog, g); glAttachShader(prog, f);
    glLinkProgram(prog);
    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512]; glGetProgramInfoLog(prog, 512, nullptr, infoLog);
        std::cerr << "Link Error: " << infoLog << std::endl;
        return 0;
    }
    return prog;
}