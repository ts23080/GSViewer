#include "Loading.h"
#include <fstream>
#include <iostream>

Loading::Loading() {}
Loading::~Loading() { m_splats.clear(); }

bool Loading::LoadFromPly(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    std::string line;
    int numVertices = 0;
    while (std::getline(file, line)) {
        if (line.find("element vertex") != std::string::npos) {
            numVertices = std::stoi(line.substr(line.find_last_of(' ')));
        }
        if (line == "end_header") break;
    }

    if (numVertices <= 0) return false;
    m_splats.resize(numVertices);

    for (int i = 0; i < numVertices; ++i) {
        GaussianSplat& s = m_splats[i];
        file.read(reinterpret_cast<char*>(&s.px), sizeof(float) * 3);  // pos
        file.read(reinterpret_cast<char*>(&s.r), sizeof(float) * 3);  // f_dc
        file.read(reinterpret_cast<char*>(s.sh_rest), sizeof(float) * 45); // f_rest
        file.read(reinterpret_cast<char*>(&s.opacity), sizeof(float)); // opacity
        file.read(reinterpret_cast<char*>(&s.sx), sizeof(float) * 3);  // scale
        file.read(reinterpret_cast<char*>(&s.rx), sizeof(float) * 4);  // rot

        if (file.fail()) {
            std::cerr << "Error: Read failed at index " << i << std::endl;
            return false;
        }
    }
    return true;
}