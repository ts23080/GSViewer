#pragma once

#include "oglforglfw.h"

class EventManager
{
  //C++11à»ç~ÇÕÇ±Ç±Ç≈èâä˙âªOK
  bool m_isL = false;
  bool m_isR = false;
  bool m_isM = false;
  EVec2i m_prepos;
  OglForGlfw m_ogl;


  bool m_b_drawstroke = false; // ó·Ç∆ÇµÇƒ3D strokeÇï`âÊÇ∑ÇÈ
  std::vector<EVec3f> m_stroke;

private: 
  EventManager();

public:
  static EventManager& getInst()
  {
    static EventManager instance;
    return instance;
  }
  
  void LBtnDown (EVec2i p, bool ctrl, bool shift);
  void RBtnDown (EVec2i p, bool ctrl, bool shift);
  void MBtnDown (EVec2i p, bool ctrl, bool shift);
  void LBtnUp   (EVec2i p, bool ctrl, bool shift);
  void RBtnUp   (EVec2i p, bool ctrl, bool shift);
  void MBtnUp   (EVec2i p, bool ctrl, bool shift);
  void MouseMove(EVec2i p);
  void MouseWheel(int dx, int dy);
  void KeyDown(int key);
  void KeyUp(int key);
  
  void DrawScene(int screen_width, int height);
};

