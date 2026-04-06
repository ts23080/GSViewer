#include "EventManager.h"
#include "math.h"


EventManager::EventManager()
{
  m_isL = false;
  m_isR = false;
  m_isM = false;
  
  m_ogl.SetCamera(EVec3f(0,0,10), EVec3f(0,0,0), EVec3f(0,1,0));
  m_ogl.SetClearColor(EVec4f(0.2f, 0.2f, 0.5f, 1.0f));
}

void EventManager::LBtnDown(EVec2i p, bool ctrl, bool shift) 
{
  m_isL = true;
  if (shift) 
  {
    m_stroke.clear();
    EVec3f pos, dir;
    m_ogl.GetCursorRay(p, pos, dir);
    m_stroke.push_back(pos); // camera位置を開始点に入れておく
    m_stroke.push_back(pos+10*dir); //奥行きは適当
    m_b_drawstroke = true;
  }
  
  m_prepos = p;
}

void EventManager::RBtnDown(EVec2i p, bool ctrl, bool shift) 
{
  m_isR = true;
  m_prepos = p;
}

void EventManager::MBtnDown(EVec2i p, bool ctrl, bool shift) 
{
  m_isM = true;
  m_prepos = p;
}

void EventManager::LBtnUp  (EVec2i p, bool ctrl, bool shift) 
{
  m_isL = false;
  m_b_drawstroke = false;
}

void EventManager::RBtnUp  (EVec2i p, bool ctrl, bool shift) 
{
  m_isR = false;
}

void EventManager::MBtnUp  (EVec2i p, bool ctrl, bool shift) 
{
  m_isM = false;
}

void EventManager::MouseMove(EVec2i p) 
{
  if (!m_isL && !m_isR && !m_isM) return;
  int dx = p.x() - m_prepos.x();
  int dy = p.y() - m_prepos.y();

  if (m_b_drawstroke)
  {
    EVec3f pos, dir;
    m_ogl.GetCursorRay(p, pos, dir);
    m_stroke.push_back(pos + 10 * dir);
    std::cout << dir[0] << "," << dir[1] << "," << dir[2] << "\n";
  }
  else 
  {
    if (m_isL)
      m_ogl.RotateCamera(dx, dy);
    else if (m_isR)
      m_ogl.TranslateCamera(dx, dy);
    else if (m_isM)
      m_ogl.ZoomCamera(dy);
  }
  m_prepos = p;
}

void EventManager::MouseWheel(int dx, int dy) {}
void EventManager::KeyDown(int key) {}
void EventManager::KeyUp(int key) {}


static void DrawFrame()
{
  glDisable(GL_LIGHTING);
  glLineWidth(2.0f);
  
  glBegin(GL_LINES);
  glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(5, 0, 0);
  glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 5, 0);
  glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 5);
  glEnd();

}

static void DrawCube()
{
  // lighting
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  float lightPos[4] = { 10,10,10,1 };
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

  // material
  float diff[4] = { 0.8f,0.5f,0.2f,1 };
  float spec[4] = { 1,1,1,1 };
  float amb[4] = { 0.2f,0.2f,0.2f,1 };
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50);
  
  //頂点 8個
  EVec3f p[8] ={{-1,-1,-1}, { 1,-1,-1}, { 1, 1,-1}, {-1, 1,-1}, 
                {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1}, {-1, 1, 1} };
  //面 6個 (頂点インデックス 4個ずつ)
  int face[6][4] = { {4,5,6,7}, {1,0,3,2}, {5,1,2,6},
                     {0,4,7,3}, {7,6,2,3}, {0,1,5,4} };
  EVec3f normal[6] = { { 0, 0, 1}, { 0, 0,-1}, { 1, 0, 0},
                       {-1, 0, 0}, { 0, 1, 0}, { 0,-1, 0}};

  glBegin(GL_QUADS);
  for (int f = 0; f < 6; ++f)
  {
    glNormal3f(normal[f].x(), normal[f].y(), normal[f].z());
    glVertex3fv(p[face[f][0]].data());
    glVertex3fv(p[face[f][1]].data());
    glVertex3fv(p[face[f][2]].data());
    glVertex3fv(p[face[f][3]].data());
  }
  glEnd();
  
}


EVec3f CalcPos(float theta, float phi)
{
  return EVec3f(
    cos(theta) * cos(phi), 
    sin(phi), 
    -sin(theta) * cos(phi));
}

void DrawSphere(const EVec3f &pos,float rad)
{
  // lighting
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  float lightPos[4] = { 10,10,10,1 };
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

  // material
  float diff[4] = { 0.0f,0.5f,0.8f,1 };
  float spec[4] = { 1,1,1,1 };
  float amb[4] = { 0.2f,0.2f,0.2f,1 };
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50);

  const int n = 30;
  
  glTranslatef(pos[0], pos[1], pos[2]);
  glBegin(GL_QUADS);
  for (int t = 0; t < n; ++t)
  {
    for (int p = 0; p < n; ++p)
    {
      float t1 =    t    / (n - 1.0f) * M_PI * 2;
      float t2 = (t + 1) / (n - 1.0f) * M_PI * 2;
      float p1 =    p    / (n - 1.0f) * M_PI - M_PI / 2;
      float p2 = (p + 1) / (n - 1.0f) * M_PI - M_PI / 2;
      
      EVec3f n0 = CalcPos(t1, p1), x0 = n0 * rad;
      EVec3f n1 = CalcPos(t2, p1), x1 = n1 * rad;
      EVec3f n2 = CalcPos(t2, p2), x2 = n2 * rad;
      EVec3f n3 = CalcPos(t1, p2), x3 = n3 * rad;
      glNormal3fv(n0.data()); glVertex3fv(x0.data());
      glNormal3fv(n1.data()); glVertex3fv(x1.data());
      glNormal3fv(n2.data()); glVertex3fv(x2.data());
      glNormal3fv(n3.data()); glVertex3fv(x3.data());
    }
  }
  glEnd();
  glTranslatef( -pos[0], -pos[1], -pos[2]);

}




void EventManager::DrawScene(
  int screen_width, 
  int screen_height) 
{
  m_ogl.OnDrawBegin(screen_width, screen_height);
  
}