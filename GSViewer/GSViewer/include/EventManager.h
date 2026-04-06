#pragma once
#include <vector>
#include <string>
#include <eigen/Dense>
#include <glad/glad.h>

#include "Loading.h"
#include "Renderer.h"

typedef Eigen::Vector2i EVec2i;
typedef Eigen::Vector3f EVec3f;
typedef Eigen::Matrix4f EMat4f;

class EventManager {
public:
    static EventManager& getInst() {
        static EventManager instance;
        return instance;
    }

    void LBtnDown(EVec2i p, bool ctrl, bool shift);
    void RBtnDown(EVec2i p, bool ctrl, bool shift);
    void MBtnDown(EVec2i p, bool ctrl, bool shift);
    void LBtnUp(EVec2i p, bool ctrl, bool shift);
    void RBtnUp(EVec2i p, bool ctrl, bool shift);
    void MBtnUp(EVec2i p, bool ctrl, bool shift);
    void MouseMove(EVec2i p);
    void MouseWheel(int dx, int dy);

    void DrawScene(int screen_width, int screen_height);

private:
    EventManager();

    // カメラ行列を計算するヘルパー関数
    EMat4f GetViewMatrix();
    EMat4f GetProjectionMatrix(float width, float height);

    bool m_isL, m_isR, m_isM;
    EVec2i m_prepos;

    // カメラパラメータ
    EVec3f m_camPos;
    float m_camYaw;   // 左右回転
    float m_camPitch; // 上下回転
    float m_camDist;  // ズーム（距離）

    Loading  m_loader;
    Renderer m_renderer;
};