#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
#include "Loading.h" // Loading::GaussianSplat 構造体を参照するため

class Renderer {
public:
    Renderer();
    ~Renderer();

    // 1. シェーダーの初期化（.vert, .geo, .frag を読み込む）
    bool InitShaders(const std::string& vPath, const std::string& gPath, const std::string& fPath);

    // 2. SSBOの作成とデータ転送
    void SetupBuffers(const Loading& loader);

    // 3. 描画実行
    void Render(int numSplats);

private:
    GLuint m_program;     // シェーダープログラムID
    GLuint m_ssbo;        // SSBO (GaussianSplatデータ用)
    GLuint m_dummyVao;    // VAO (描画命令のキッカケ用)

    // シェーダーコンパイル用の補助関数
    GLuint CompileShader(GLenum type, const std::string& path);
};
