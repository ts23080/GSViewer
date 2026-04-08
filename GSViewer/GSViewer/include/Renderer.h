#pragma once

#include <glad/glad.h>
#include <vector>
#include "Loading.h" // GaussianSplat 構造体の定義を使用するため

class Renderer {
public:
    Renderer();
    ~Renderer();

    /**
     * @brief 頂点データ(VBO/VAO)の初期化とシェーダープログラムの登録
     * @param splats Loadingクラスで読み込んだデータのベクター
     * @param program コンパイル済みのシェーダープログラムID
     */
    void Setup(const std::vector<Loading::GaussianSplat>& splats, GLuint program);

    /**
     * @brief 描画実行
     * @param numSplats 描画するポイントの数
     * @param view ビュー行列の先頭ポインタ (float[16])
     * @param proj 投影行列の先頭ポインタ (float[16])
     * @param screenW 画面幅 (不透明度やサイズの計算用)
     * @param screenH 画面高さ
     */
    void Render(int numSplats, const float* view, const float* proj, int screenW, int screenH);

private:
    GLuint m_vao;       // Vertex Array Object
    GLuint m_vbo;       // Vertex Buffer Object
    GLuint m_program;   // Shader Program ID

    // コピー禁止（リソース管理のため）
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
};