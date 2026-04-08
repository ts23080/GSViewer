#pragma once
#include <glad/glad.h>
#include <Eigen/Dense>  // Eigenを必ずインクルード
#include <vector>
#include "Loading.h"
#include "Renderer.h"

// --- ここでエイリアスを定義 ---
typedef Eigen::Matrix4f EMat4f;
typedef Eigen::Vector3f EVec3f;
typedef Eigen::Vector2i EVec2i;

class EventManager {
public:
    static EventManager& getInst();

    // 引数や戻り値にエイリアスを使用
    void DrawScene(int w, int h);
    void MouseMove(EVec2i p);
    void MouseWheel(int dx, int dy);
    void LBtnDown(EVec2i p, bool c, bool s);
    void LBtnUp(EVec2i p, bool c, bool s);
    void RBtnDown(EVec2i p, bool c, bool s);
    void RBtnUp(EVec2i p, bool c, bool s);
    void MBtnDown(EVec2i p, bool c, bool s);
    void MBtnUp(EVec2i p, bool c, bool s);

private:
    EventManager();
    ~EventManager() = default;

    void SortSplats();
    EMat4f GetViewMatrix();
    EMat4f GetProjectionMatrix(float w, float h);

    Loading m_loader;
    Renderer m_renderer;
    std::vector<unsigned int> m_indices;

    EVec3f m_camPos; // EVec3fを使用
    float m_camYaw, m_camPitch, m_camDist;
    EVec2i m_prepos; // EVec2iを使用
    bool m_isL, m_isR, m_isM;
};