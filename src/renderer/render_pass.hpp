#pragma once
#include "framebuffer.hpp"
#include "shader.h"
#include "vec.hpp"

namespace fluidity
{

struct RenderState
{
  bool useDepthTest = true;
  bool useBlend = false;
  GLenum blendSourceFactor = GL_ONE;
  GLenum blendDestinationFactor = GL_ZERO;
  Vec4 clearColor = { 0.f, 0.f, 0.f, 1.f };
};

class RenderPass
{
public:
  RenderPass(
    int bufferWidth,
    int bufferHeight,
    int numberOfParticles,
    const float pointRadius,
    GLuint particlesVAO
  );
  virtual ~RenderPass() { };

  virtual bool Init()   = 0;
  virtual void Render() = 0;
  virtual GLuint GetBuffer() { return m_framebuffer.GetAttachment(0); }

  virtual void SetParticlesVAO(GLuint particlesVAO)      { m_particlesVAO      = particlesVAO; }
  virtual void SetNumberOfParticles(unsigned nParticles) { m_numberOfParticles = nParticles; }
  virtual bool SetUniformBuffer(const std::string& name, GLuint uniformBlockBinding);

  Shader& GetShader();
  Framebuffer& GetFramebuffer() { return m_framebuffer; }

  virtual void SetRenderState(const RenderState& state) { m_renderState = state; };
  virtual const RenderState& GetRenderState() { return m_renderState; } 

protected:
  virtual void ChangeOpenGLRenderState(const RenderState& state);
  virtual RenderState GetCurrentOpenGLRenderState();

  virtual bool SetUniforms() { return true; }
  Shader* m_shader; 
  unsigned m_bufferWidth;
  unsigned m_bufferHeight;
  unsigned m_numberOfParticles;
  float m_pointRadius;

  GLuint m_particlesVAO;
  Framebuffer m_framebuffer;
  RenderState m_renderState;
};

}
