#include "EventManager.h"
#include <iostream>
#include <algorithm>

EventManager::EventManager()
    : m_isL(false), m_isR(false), m_isM(false),
    m_camPos(0, 0, 0), m_camYaw(0), m_camPitch(0), m_camDist(5.0f)
{
    if (m_loader.LoadFromPly("model.ply")) {
        m_renderer.Init("splat.vert", "splat.geo", "splat.frag");
        m_renderer.SetupBuffers(m_loader);
    }
}

// 視点行列の作成 (LookAt)
EMat4f EventManager::GetViewMatrix() {
    // 極座標からカメラ位置を算出
    float x = m_camDist * cos(m_camPitch) * sin(m_camYaw);
    float y = m_camDist * sin(m_camPitch);
    float z = m_camDist * cos(m_camPitch) * cos(m_camYaw);

    EVec3f eye = m_camPos + EVec3f(x, y, z);
    EVec3f center = m_camPos;
    EVec3f up(0, 1, 0);

    EVec3f f = (center - eye).normalized();
    EVec3f s = f.cross(up).normalized();
    EVec3f u = s.cross(f);

    EMat4f res = EMat4f::Identity();
    res(0, 0) = s.x(); res(0, 1) = s.y(); res(0, 2) = s.z();
    res(1, 0) = u.x(); res(1, 1) = u.y(); res(1, 2) = u.z();
    res(2, 0) = -f.x(); res(2, 1) = -f.y(); res(2, 2) = -f.z();
    res(0, 3) = -s.dot(eye);
    res(1, 3) = -u.dot(eye);
    res(2, 3) = f.dot(eye);
    return res;
}

// 透視投影行列の作成
EMat4f EventManager::GetProjectionMatrix(float w, float h) {
    float aspect = w / h;
    float fov = 45.0f * 3.14159265f / 180.0f;
    float zNear = 0.1f;
    float zFar = 1000.0f;
    float tanHalfFov = tan(fov / 2.0f);

    EMat4f res = EMat4f::Zero();
    res(0, 0) = 1.0f / (aspect * tanHalfFov);
    res(1, 1) = 1.0f / (tanHalfFov);
    res(2, 2) = -(zFar + zNear) / (zFar - zNear);
    res(2, 3) = -(2.0f * zFar * zNear) / (zFar - zNear);
    res(3, 2) = -1.0f;
    return res;
}

void EventManager::DrawScene(int w, int h) {
    glViewport(0, 0, w, h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    EMat4f view = GetViewMatrix();
    EMat4f proj = GetProjectionMatrix((float)w, (float)h);

    // Rendererへ行列を渡す (Eigenのdata()は列優先なのでそのまま渡せる)
    m_renderer.Render(m_loader.GetNumSplats(), view.data(), proj.data(), w, h);
}

// --- マウス操作でのパラメータ更新 ---

void EventManager::MouseMove(EVec2i p) {
    if (!m_isL && !m_isR && !m_isM) return;
    float dx = (float)(p.x() - m_prepos.x());
    float dy = (float)(p.y() - m_prepos.y());

    if (m_isL) { // 回転
        m_camYaw -= dx * 0.01f;
        m_camPitch += dy * 0.01f;
        m_camPitch = std::max(-1.5f, std::min(1.5f, m_camPitch)); // 首振り制限
    }
    else if (m_isR) { // 平行移動
        m_camPos.y() += dy * 0.01f;
        m_camPos.x() -= dx * 0.01f;
    }
    else if (m_isM) { // ズーム
        m_camDist += dy * 0.1f;
    }
    m_prepos = p;
}

void EventManager::MouseWheel(int dx, int dy) {
    m_camDist -= (float)dy * 0.5f;
    if (m_camDist < 0.1f) m_camDist = 0.1f;
}

// 残りのボタンイベント（前回のものと同じ）
void EventManager::LBtnDown(EVec2i p, bool c, bool s) { m_isL = true; m_prepos = p; }
void EventManager::RBtnDown(EVec2i p, bool c, bool s) { m_isR = true; m_prepos = p; }
void EventManager::MBtnDown(EVec2i p, bool c, bool s) { m_isM = true; m_prepos = p; }
void EventManager::LBtnUp(EVec2i p, bool c, bool s) { m_isL = false; }
void EventManager::RBtnUp(EVec2i p, bool c, bool s) { m_isR = false; }
void EventManager::MBtnUp(EVec2i p, bool c, bool s) { m_isM = false; }