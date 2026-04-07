#pragma once

#include <glad/glad.h>
#include <eigen/Dense>
#include <memory>
#include <algorithm>

#include "Loading.h"
#include "Renderer.h"

// 型名のエイリアス (cpp側の記述に合わせる)
using EVec2i = Eigen::Vector2i;
using EVec3f = Eigen::Vector3f;
using EMat4f = Eigen::Matrix4f;

class EventManager {
public:
    // シングルトンインスタンスの取得
    static EventManager& getInst() {
        static EventManager instance;
        return instance;
    }

    // 描画メインルーチン
    void DrawScene(int w, int h);

    // マウス・キーイベント
    void MouseMove(EVec2i p);
    void MouseWheel(int dx, int dy);
    void LBtnDown(EVec2i p, bool c, bool s);
    void RBtnDown(EVec2i p, bool c, bool s);
    void MBtnDown(EVec2i p, bool c, bool s);
    void LBtnUp(EVec2i p, bool c, bool s);
    void RBtnUp(EVec2i p, bool c, bool s);
    void MBtnUp(EVec2i p, bool c, bool s);

private:
    // コンストラクタ・デストラクタ
    EventManager();
    ~EventManager() = default;

    // コピー禁止
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    // 行列計算ヘルパー
    EMat4f GetViewMatrix();
    EMat4f GetProjectionMatrix(float w, float h);

    // メンバ変数
    Loading  m_loader;
    Renderer m_renderer;

    // カメラパラメータ
    EVec3f m_camPos;    // 注視点のオフセット
    float  m_camYaw;    // 回転角（左右）
    float  m_camPitch;  // 回転角（上下）
    float  m_camDist;   // カメラの距離

    // マウス状態
    EVec2i m_prepos;    // 直前のマウス位置
    bool   m_isL;       // 左ボタン押下状態
    bool   m_isR;       // 右ボタン押下状態
    bool   m_isM;       // 中ボタン押下状態
};