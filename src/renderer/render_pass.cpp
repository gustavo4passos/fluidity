#include "render_pass.hpp"
#include "../utils/glcall.h"

namespace fluidity
{
RenderPass::RenderPass(
  const unsigned bufferWidth,
  const unsigned bufferHeight,
  const unsigned numberOfParticles,
  const float pointRadius,
  GLuint particlesVAO)
  : m_bufferWidth(bufferWidth),
  m_bufferHeight(bufferHeight),
  m_numberOfParticles(numberOfParticles),
  m_pointRadius(pointRadius),
  m_particlesVAO(particlesVAO),
  m_framebuffer({})
{ /* */ } 

bool RenderPass::Init()
{
    if (!m_framebuffer.Init()) return false;
    if (!SetUniforms())       return false;
    return true;
}

bool RenderPass::SetUniformBuffer(const std::string& name, GLuint uniformBlockBinding)
{
  GLuint index = glGetUniformBlockIndex(m_shader->programID(), name.c_str());
  if (index == GL_INVALID_INDEX) return false;

  GLCall(glUniformBlockBinding(m_shader->programID(), index, 
        uniformBlockBinding));
  
  return true;
}
}
