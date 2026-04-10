#include "Loading.h"
#include <fstream>
#include <iostream>
#include <string>

bool Loading::LoadFromPly(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open PLY file: " << filename << std::endl;
        return false;
    }

    std::string line;
    int numVertices = 0;
    bool headerEnded = false;

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

    m_splats.clear();
    m_splats.resize(numVertices);

    for (int i = 0; i < numVertices; ++i) {
        Splat::GaussianSplat& s = m_splats[i];

        file.read(reinterpret_cast<char*>(&s.px), sizeof(float) * 3);
        file.read(reinterpret_cast<char*>(&s.r), sizeof(float) * 3);
        file.read(reinterpret_cast<char*>(s.sh_rest), sizeof(float) * 45);
        file.read(reinterpret_cast<char*>(&s.opacity), sizeof(float));
        file.read(reinterpret_cast<char*>(&s.sx), sizeof(float) * 3);
        float q[4];
        file.read(reinterpret_cast<char*>(q), sizeof(float) * 4);
        s.rw = q[0]; // w¬•ª
        s.rx = q[1]; // x¬•ª
        s.ry = q[2]; // y¬•ª
        s.rz = q[3]; // z¬•ª

        if (file.fail()) {
            std::cerr << "Error: Binary data ended prematurely at index " << i << std::endl;
            return false;
        }
    }

    std::cout << "Loading: Successfully loaded " << numVertices << " Gaussian Splats." << std::endl;

    if (numVertices > 0) {
        std::cout << "Debug Point[0] Pos: (" << m_splats[0].px << ", "
            << m_splats[0].py << ", " << m_splats[0].pz << ")" << std::endl;
    }

    return true;
}