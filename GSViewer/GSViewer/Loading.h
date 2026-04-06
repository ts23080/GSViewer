#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>

class Loading {
public:
    struct GaussianSplat {
        float pos[3];      // 0-2: x, y, z
        float sh_base[3];  // 3-5: f_dc_0, 1, 2
        float sh_rest[48]; // 6-53: f_rest_0...47 (色の変化)
        float opacity;     // 54: 不透明度
        float scale[3];    // 55-57: スケール
        float rot[4];      // 58-61: 回転 (Quaternion)
    };

    Loading();
    ~Loading();

    bool LoadFromPly(const std::string& filename);

    const std::vector<GaussianSplat>& GetSplats() const { return m_splats; }
    int GetNumSplats() const { return static_cast<int>(m_splats.size()); }

private:
    std::vector<GaussianSplat> m_splats;
};