#include "Loading.h"
#include <fstream>
#include <iostream>
#include <string>

// Loading::Loading() {} // コンストラクタはヘッダーで定義済みなら不要（二重定義注意）
// もしヘッダーで定義していない場合は以下を生かしてください
// Loading::Loading() {}

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
    for (int i = 0; i < numVertices; ++i) {
        // Splat:: 名前空間を明示的に指定
        Splat::GaussianSplat& s = m_splats[i];

        // 位置 (px, py, pz)
        file.read(reinterpret_cast<char*>(&s.px), sizeof(float) * 3);

        // SH基礎 (r, g, b)
        file.read(reinterpret_cast<char*>(&s.r), sizeof(float) * 3);

        // SH詳細 (sh_rest)
        file.read(reinterpret_cast<char*>(s.sh_rest), sizeof(float) * 45);

        // 不透明度 (opacity)
        file.read(reinterpret_cast<char*>(&s.opacity), sizeof(float));

        // スケール (sx, sy, sz)
        file.read(reinterpret_cast<char*>(&s.sx), sizeof(float) * 3);

        // 6. 回転 (w, x, y, z で読み込み、構造体の適切なメンバへ格納)
        float q[4]; // q[0]=w, q[1]=x, q[2]=y, q[3]=z
        file.read(reinterpret_cast<char*>(q), sizeof(float) * 4);

        // 構造体のメンバ名(rx,ry,rz,rw)に合わせて意味を固定
        s.rw = q[0]; // w成分
        s.rx = q[1]; // x成分
        s.ry = q[2]; // y成分
        s.rz = q[3]; // z成分

        // 読み込みエラーチェック
        if (file.fail()) {
            std::cerr << "Error: Binary data ended prematurely at index " << i << std::endl;
            return false;
        }
    }

    std::cout << "Loading: Successfully loaded " << numVertices << " Gaussian Splats." << std::endl;

    // デバッグ確認
    if (numVertices > 0) {
        std::cout << "Debug Point[0] Pos: (" << m_splats[0].px << ", "
            << m_splats[0].py << ", " << m_splats[0].pz << ")" << std::endl;
    }

    return true;
}