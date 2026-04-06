#pragma once

//OpenGL 
#include <glad/glad.h>
#include <GLFW/glfw3.h>

//Eigen 
#include <eigen/Dense>
#include <eigen/Geometry> 

//stl
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <deque>

typedef Eigen::Vector2i EVec2i;
typedef Eigen::Vector2f EVec2f;
typedef Eigen::Vector3i EVec3i;
typedef Eigen::Vector3f EVec3f;
typedef Eigen::Vector4i EVec4i;
typedef Eigen::Vector4f EVec4f;
typedef Eigen::Matrix3f EMat3f;
typedef Eigen::Matrix4f EMat4f;
typedef Eigen::VectorXf EVecXf;
typedef Eigen::MatrixXf EMatXf;

#ifndef byte
typedef unsigned char byte;
#endif

#ifndef max3
#define max3(a,b,c)      std::max((a), std::max( (b),(c) ))
#endif

#ifndef min3
#define min3(a,b,c)      std::min((a), std::min( (b),(c) ))
#endif

#ifndef max3id
#define max3id(a,b,c)    ((a>=b&&a>=c)?0:(b>=c)?1:2)
#endif

#ifndef min3id
#define min3id(a,b,c)    ((a<=b&&a<=c)?0:(b<=c)?1:2)
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


class OglCamera
{
public:
  EVec3f m_pos; //camera position
  EVec3f m_cnt; //camera focus point
  EVec3f m_up;  //camera Up (y-axis) direction 

  OglCamera()
  {
    m_pos = EVec3f(0, 0, 10);
    m_cnt = EVec3f(0, 0, 0);
    m_up = EVec3f(0, 1, 0);
  }

  OglCamera(const OglCamera& src)
  {
    Copy(src);
  }

  OglCamera& operator=(const OglCamera& src)
  {
    Copy(src);
    return *this;
  }

  void Copy(const OglCamera& src)
  {
    m_pos = src.m_pos;
    m_cnt = src.m_cnt;
    m_up = src.m_up;
  }

  //camera rotation/zoom/translate by mouse operation 
  // (mouse_dx, mouse_dy) : mouse offsets in 2D
  //  
  void RotateCamera(int mouse_dx, int mouse_dy)
  {
    float theta = -mouse_dx / 200.0f;
    float phi = -mouse_dy / 200.0f;

    EVec3f x_dir = ((m_cnt - m_pos).cross(m_up)).normalized();
    Eigen::AngleAxisf rotTheta(theta, m_up);
    Eigen::AngleAxisf rotPhi(phi, x_dir);
    m_up = rotPhi * rotTheta * m_up;
    m_pos = rotPhi * rotTheta * (m_pos - m_cnt) + m_cnt;
  }

  void ZoomCamera(int mouse_dy)
  {
    EVec3f newEyeP = m_pos + mouse_dy / 80.0f * (m_cnt - m_pos);
    if ((newEyeP - m_cnt).norm() > 0.02f) m_pos = newEyeP;
  }

  void TranslateCamera(int mouse_dx, int mouse_dy)
  {
    float c = (m_pos - m_cnt).norm() / 900.0f;
    EVec3f x_dir = ((m_pos - m_cnt).cross(m_up)).normalized();
    EVec3f t = c * mouse_dx * x_dir + c * mouse_dy * m_up;
    m_pos += t;
    m_cnt += t;
  }

  void ZoomCameraByOffset(float offset)
  {
    EVec3f rayD = (m_cnt - m_pos);
    float  len = rayD.norm();
    if (offset > len) return;
    rayD /= len;
    m_pos = m_pos + offset * rayD;
  }

  void SetCamera(const EVec3f& pos, const EVec3f& cnt, const EVec3f up)
  {
    m_pos = pos;
    m_cnt = cnt;
    m_up = up;
  }


  //make LookAt matrix
  //óÒmajorÇ»ÇÃÇ≈ÇªÇÃÇ‹Ç‹ìnÇπÇÈ
  EMat4f MakeLookAtMat() const
  {
    EVec3f f = (m_cnt - m_pos).normalized();
    EVec3f s = f.cross(m_up).normalized();
    EVec3f u = s.cross(f);

    EMat4f M = EMat4f::Identity();

    M(0, 0) =  s.x(); M(0, 1) =  s.y(); M(0, 2) =  s.z(); M(0, 3) = -s.dot(m_pos);
    M(1, 0) =  u.x(); M(1, 1) =  u.y(); M(1, 2) =  u.z(); M(1, 3) = -u.dot(m_pos);
    M(2, 0) = -f.x(); M(2, 1) = -f.y(); M(2, 2) = -f.z(); M(2, 3) =  f.dot(m_pos);

    return M;
  }



};


inline EMat4f MakePerspectiveMat(
  float fovyDeg,
  float aspect,
  float nearZ,
  float farZ)
{
  float f = 1.f / std::tan(fovyDeg * float(M_PI) / 360.f);

  EMat4f P = Eigen::Matrix4f::Zero();
  P(0, 0) = f / aspect;
  P(1, 1) = f;
  P(2, 2) = (farZ + nearZ) / (nearZ - farZ);
  P(2, 3) = (2.f * farZ * nearZ) / (nearZ - farZ);
  P(3, 2) = -1.f;

  return P;
}



class OglForGlfw
{
private:
  OglCamera m_camera;
  
  EVec4f m_clear_color;
  int m_screen_width;
  int m_screen_height;
  float m_near, m_far;
  float m_fovY;

public:
  OglForGlfw()
  {
    m_screen_width = 640;
    m_screen_height = 480;
    m_near = 0.1f;
    m_far = 100.0f;
    m_fovY = 45.0f;
    m_clear_color = EVec4f(0.0f, 0.0f, 0.0f, 1.0f);
  }

  void SetCamera(EVec3f pos, EVec3f cnt, EVec3f up)
  {
    m_camera.SetCamera(pos, cnt, up);
  }

  void SetFovY(float fovY)
  {
    m_fovY = fovY;
  }

  void SetNearFar(float near, float far)
  {
    m_near = near;
    m_far = far;
  }
  
  void SetClearColor(const EVec4f& color)
  {
    m_clear_color = color;
  }

  void RotateCamera(int mouse_dx, int mouse_dy)
  {
    m_camera.RotateCamera(mouse_dx, mouse_dy);
  }

  void ZoomCamera(int mouse_dy)
  {
    m_camera.ZoomCamera(mouse_dy);
  }

  void TranslateCamera(int mouse_dx, int mouse_dy)
  {
    m_camera.TranslateCamera(mouse_dx, mouse_dy);
  }

  void ZoomCameraByOffset(float offset)
  {
    m_camera.ZoomCameraByOffset(offset);
  }


  //call at begin of draw
  void OnDrawBegin(int screen_w, int screen_h)
  {
    m_screen_width = screen_w;
    m_screen_height = screen_h;

    glViewport(0, 0, m_screen_width, m_screen_height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(m_clear_color.x(), 
                 m_clear_color.y(), 
                 m_clear_color.z(), m_clear_color.w());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    EMat4f P = MakePerspectiveMat(m_fovY, float(m_screen_width) / float(m_screen_height), m_near, m_far);
    EMat4f V = m_camera.MakeLookAtMat();
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(P.data());
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(V.data());
  }

  //Screenè„ÇÃç¿ïWÇ3DãÛä‘ÇÃÉåÉCÇ…ïœä∑
  //posÇÕcamera position
  void GetCursorRay(EVec2i p, EVec3f& pos, EVec3f& dir)
  {
    float nx = (2.0f * p[0]) / m_screen_width - 1.0f;
    float ny = (2.0f * p[1]) / m_screen_height - 1.0f;
    ny = -ny;
    float aspect = float(m_screen_width) / float(m_screen_height);

    EMat4f P = MakePerspectiveMat(m_fovY, aspect, m_near, m_far);
    EMat4f V = m_camera.MakeLookAtMat();

    EMat4f PV_inv = (P * V).inverse();

    float depth = 0.01f; 
    EVec4f wp = PV_inv * EVec4f(nx, ny, depth, 1.0f);

    EVec3f targetPos(wp[0] / wp[3], wp[1] / wp[3], wp[2] / wp[3]);

    pos = m_camera.m_pos;
    dir = (targetPos - pos).normalized();
  }

};