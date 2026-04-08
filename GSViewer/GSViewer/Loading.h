#pragma once
#include <vector>
#include <string>

namespace Splat {
    // GPUに送るメモリレイアウトと完全に一致させる構造体
    struct GaussianSplat {
        float px, py, pz;      // 位置
        float r, g, b;         // 色（SH基本項）
        float sh_rest[45];     // SH残り（高次項）
        float opacity;         // 不透明度
        float sx, sy, sz;      // スケール
        float rx, ry, rz, rw;  // 回転（クォータニオン）
    };
}

class Loading {
public: // ★ここが抜けていると外部（EventManager等）から関数を呼べません
    Loading() {}

    // PLYファイルから m_splats（メンバ変数）へデータを読み込む
    bool LoadFromPly(const std::string& filename);

    // ゲッター関数
    // 戻り値の型にも Splat:: を付ける必要があります
    const std::vector<Splat::GaussianSplat>& GetSplats() const { return m_splats; }

    int GetNumSplats() const { return static_cast<int>(m_splats.size()); }

private:
    // 構造体の配列としてメンバ変数で管理
    std::vector<Splat::GaussianSplat> m_splats;
};