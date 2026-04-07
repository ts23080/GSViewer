#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>

class Loading {
public:
    struct GaussianSplat {
        float px, py, pz;         // 位置
        float r, g, b;            // SH基礎 (f_dc)
        float sh_rest[45];        // SH詳細 (f_rest_0 ～ 44)
        float opacity;            // 不透明度
        float sx, sy, sz;         // スケール
        float rx, ry, rz, rw;     // 回転 (Quaternion)
    };

    Loading();
    ~Loading();

    bool LoadFromPly(const std::string& filename);
    const std::vector<GaussianSplat>& GetSplats() const { return m_splats; }
    int GetNumSplats() const { return static_cast<int>(m_splats.size()); }

private:
    std::vector<GaussianSplat> m_splats;
};