// FirstOglGlfw.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <eigen/Dense>
#include "EventManager.h"


// DLL版GLFWを使う と OpenGLを使う準備
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "opengl32.lib")


class DataManager
{
  private :
    DataManager(){}

public:
  static DataManager& GetInstance()
  {
    static DataManager p;
    return p;
  }

};





static void mouseButtonCallback(GLFWwindow * w, int button, int action, int mods)
{
  double x, y;
  glfwGetCursorPos(w, &x, &y);
  EVec2i p(static_cast<int>(x), static_cast<int>(y));
  const bool ctrl = (mods & GLFW_MOD_CONTROL) != 0;
  const bool shift = (mods & GLFW_MOD_SHIFT  ) != 0;

  EventManager& em = EventManager::getInst();
  if (action == GLFW_PRESS) 
  {
    if (button == GLFW_MOUSE_BUTTON_LEFT  ) em.LBtnDown(p, ctrl, shift);
    if (button == GLFW_MOUSE_BUTTON_RIGHT ) em.RBtnDown(p, ctrl, shift);
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) em.MBtnDown(p, ctrl, shift);
  }
  if (action == GLFW_RELEASE)
  {
    if (button == GLFW_MOUSE_BUTTON_LEFT  ) em.LBtnUp(p, ctrl, shift);
    if (button == GLFW_MOUSE_BUTTON_RIGHT ) em.RBtnUp(p, ctrl, shift);
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) em.MBtnUp(p, ctrl, shift);
  }
}

static void cursorPosCallback(GLFWwindow* w, double x, double y)
{
  EVec2i p(static_cast<int>(x), static_cast<int>(y));
  EventManager::getInst().MouseMove(p);
}

static void scrollCallback(GLFWwindow* w, double dx, double dy)
{
  EventManager::getInst().MouseWheel(static_cast<int>(dx), static_cast<int>(dy));
  std::cout << "MouseWheel " << dx << " " << dy << "\n";
}


static void keyCallback(GLFWwindow*, int key, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS)   
    std::cout << "KeyDown " << key << "\n";
  if (action == GLFW_RELEASE) 
    std::cout << "KeyUp   " << key << "\n";
}





int main()
{
  std::cout << "Hello World!\n";
  glfwInit();

  //Opengl2.1 指定 -- ここを3.3にすると modann openglになり即時モード利用不可
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); 

  GLFWwindow* w = glfwCreateWindow(640, 480, "GLAD OK", nullptr, nullptr);
  glfwMakeContextCurrent(w);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); //これでgl関数が使えるように

  std::cout << "OpenGL: " << reinterpret_cast<const char*>(glGetString(GL_VERSION)) << "\n";
  glfwSetMouseButtonCallback(w, mouseButtonCallback);
  glfwSetCursorPosCallback  (w, cursorPosCallback  );
  glfwSetScrollCallback     (w, scrollCallback     );
  glfwSetKeyCallback        (w, keyCallback);

  while (!glfwWindowShouldClose(w))
  {
    int fbw, fbh;
    glfwGetFramebufferSize(w, &fbw, &fbh);
    EventManager::getInst().DrawScene(fbw, fbh);

    glfwPollEvents();
    glfwSwapBuffers(w);
  }

  glfwTerminate();

}




