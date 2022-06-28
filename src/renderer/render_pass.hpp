#pragma once
#include "framebuffer.hpp"
#include "shader.h"

namespace fluidity
{

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

protected:
  virtual bool SetUniforms() { return true; }
  Shader* m_shader; 
  unsigned m_bufferWidth;
  unsigned m_bufferHeight;
  unsigned m_numberOfParticles;
  float m_pointRadius;

  GLuint m_particlesVAO;
  Framebuffer m_framebuffer;
};

}
