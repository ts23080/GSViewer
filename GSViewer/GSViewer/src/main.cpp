#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <eigen/Dense>
#include "EventManager.h"

// DLL版GLFWとOpenGLを使う準備
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "opengl32.lib")

// コールバック関数群（変更なし）
static void mouseButtonCallback(GLFWwindow* w, int button, int action, int mods) {
    double x, y;
    glfwGetCursorPos(w, &x, &y);
    EVec2i p(static_cast<int>(x), static_cast<int>(y));
    const bool ctrl = (mods & GLFW_MOD_CONTROL) != 0;
    const bool shift = (mods & GLFW_MOD_SHIFT) != 0;

    EventManager& em = EventManager::getInst();
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) em.LBtnDown(p, ctrl, shift);
        if (button == GLFW_MOUSE_BUTTON_RIGHT) em.RBtnDown(p, ctrl, shift);
        if (button == GLFW_MOUSE_BUTTON_MIDDLE) em.MBtnDown(p, ctrl, shift);
    }
    else if (action == GLFW_RELEASE) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) em.LBtnUp(p, ctrl, shift);
        if (button == GLFW_MOUSE_BUTTON_RIGHT) em.RBtnUp(p, ctrl, shift);
        if (button == GLFW_MOUSE_BUTTON_MIDDLE) em.MBtnUp(p, ctrl, shift);
    }
}

static void cursorPosCallback(GLFWwindow* w, double x, double y) {
    EVec2i p(static_cast<int>(x), static_cast<int>(y));
    EventManager::getInst().MouseMove(p);
}

static void scrollCallback(GLFWwindow* w, double dx, double dy) {
    EventManager::getInst().MouseWheel(static_cast<int>(dx), static_cast<int>(dy));
}

static void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) std::cout << "KeyDown " << key << "\n";
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // --- OpenGL 4.6 Core Profile 指定 ---
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 最近のMac等でも動作するように
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* w = glfwCreateWindow(1280, 720, "Gaussian Splatting Renderer 4.6", nullptr, nullptr);
    if (!w) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(w);

    // --- GLADのロード (OpenGL関数が使用可能になる) ---
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";

    // --- EventManagerの初期化 ---
    // ※必ず gladLoadGLLoader の後に取得してください。
    // インスタンス化のタイミングでシェーダーのコンパイル等が行われます。
    EventManager& em = EventManager::getInst();

    // コールバックの設定
    glfwSetMouseButtonCallback(w, mouseButtonCallback);
    glfwSetCursorPosCallback(w, cursorPosCallback);
    glfwSetScrollCallback(w, scrollCallback);

    glfwSwapInterval(1);

    // アルファブレンディングを有効化
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(w)) {
        int fbw, fbh;
        glfwGetFramebufferSize(w, &fbw, &fbh);

        // 描画実行
        em.DrawScene(fbw, fbh);

        glfwSwapBuffers(w);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}