#include "Loading.h"
#include <fstream>
#include <iostream>
#include <string>

Loading::Loading() {}
Loading::~Loading() { m_splats.clear(); }

bool Loading::LoadFromPly(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open PLY file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int numVertices = 0;

    // 1. ヘッダーを解析して頂点数(element vertex)を探す
    while (std::getline(file, line)) {
        if (line.find("element vertex") != std::string::npos) {
            size_t pos = line.find_last_of(' ');
            numVertices = std::stoi(line.substr(pos));
        }
        if (line == "end_header") break;
    }

    if (numVertices <= 0) {
        std::cerr << "Error: No vertices found in PLY header." << std::endl;
        return false;
    }

    // 2. メンバ変数のベクターをリサイズして準備
    m_splats.resize(numVertices);

    // 3. バイナリデータを順番に読み込む
    for (int i = 0; i < numVertices; ++i) {
        GaussianSplat& s = m_splats[i];

        // 位置 (x, y, z)
        file.read(reinterpret_cast<char*>(s.pos), sizeof(float) * 3);

        // 法線 (nx, ny, nz) 読み飛ばし
        float dummyNormal[3];
        file.read(reinterpret_cast<char*>(dummyNormal), sizeof(float) * 3);

        // SH基礎 (f_dc 0,1,2)
        file.read(reinterpret_cast<char*>(s.sh_base), sizeof(float) * 3);

        // SH詳細 (f_rest 0~48)
        file.read(reinterpret_cast<char*>(s.sh_rest), sizeof(float) * 48);

        // 不透明度
        file.read(reinterpret_cast<char*>(&s.opacity), sizeof(float));

        // スケール
        file.read(reinterpret_cast<char*>(s.scale), sizeof(float) * 3);

        // 回転 (Quaternion)
        file.read(reinterpret_cast<char*>(s.rot), sizeof(float) * 4);
    }

    std::cout << "Loading Class: Successfully loaded " << numVertices << " splats." << std::endl;
    return true;
}