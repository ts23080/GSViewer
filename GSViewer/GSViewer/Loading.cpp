#include "Loading.h"
#include <fstream>
#include <iostream>
#include <string>

Loading::Loading() {}
Loading::~Loading() { m_splats.clear(); }

bool Loading::LoadFromPly(const std::string& filename) {
    // 1. バイナリモードでファイルを開く
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open PLY file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int numVertices = 0;
    bool headerEnded = false;

    // 2. ヘッダー解析 (頂点数 element vertex を取得)
    while (std::getline(file, line)) {
        // Windowsの改行コード \r を除去
        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (line.find("element vertex") != std::string::npos) {
            numVertices = std::stoi(line.substr(line.find_last_of(' ') + 1));
        }
        if (line == "end_header") {
            headerEnded = true;
            break;
        }
    }

    if (!headerEnded || numVertices <= 0) {
        std::cerr << "Error: Invalid PLY header or no vertices found." << std::endl;
        return false;
    }

    // 3. メモリ確保
    m_splats.clear();
    m_splats.resize(numVertices);

    // 4. バイナリデータの読み込み
    // ヘッダーで定義された GaussianSplat の構造通りに読み込みます
    for (int i = 0; i < numVertices; ++i) {
        GaussianSplat& s = m_splats[i];

        // 位置 (px, py, pz) : 3 floats
        file.read(reinterpret_cast<char*>(&s.px), sizeof(float) * 3);

        // SH基礎 (r, g, b) : 3 floats (f_dc_0, 1, 2)
        file.read(reinterpret_cast<char*>(&s.r), sizeof(float) * 3);

        // SH詳細 (sh_rest) : 45 floats (f_rest_0 ～ 44)
        file.read(reinterpret_cast<char*>(s.sh_rest), sizeof(float) * 45);

        // 不透明度 (opacity) : 1 float
        file.read(reinterpret_cast<char*>(&s.opacity), sizeof(float));

        // スケール (sx, sy, sz) : 3 floats
        file.read(reinterpret_cast<char*>(&s.sx), sizeof(float) * 3);

        // 回転 (rx, ry, rz, rw) : 4 floats
        file.read(reinterpret_cast<char*>(&s.rx), sizeof(float) * 4);

        // 読み込みエラーチェック
        if (file.fail()) {
            std::cerr << "Error: Binary data ended prematurely at index " << i << std::endl;
            return false;
        }
    }

    std::cout << "Loading: Successfully loaded " << numVertices << " Gaussian Splats." << std::endl;

    // デバッグ確認用（最初の1点の座標を表示）
    if (numVertices > 0) {
        std::cout << "Debug Point[0] Pos: (" << m_splats[0].px << ", "
            << m_splats[0].py << ", " << m_splats[0].pz << ")" << std::endl;
    }

    return true;
}