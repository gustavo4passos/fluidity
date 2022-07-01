#include "render_pass.hpp"
#include "../utils/glcall.h"
#include <cassert>

namespace fluidity
{
RenderPass::RenderPass(
  int bufferWidth,
  int bufferHeight,
  int numberOfParticles,
  const float pointRadius,
  GLuint particlesVAO)
  : m_bufferWidth(bufferWidth),
  m_bufferHeight(bufferHeight),
  m_numberOfParticles(numberOfParticles),
  m_pointRadius(pointRadius),
  m_particlesVAO(particlesVAO),
  m_framebuffer({ {}, bufferWidth, bufferHeight }),
  m_renderState({})
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

void RenderPass::ChangeOpenGLRenderState(const RenderState& state)
{
  if (state.useDepthTest) glEnable(GL_DEPTH_TEST);
  else glDisable(GL_DEPTH_TEST);

  if (state.useBlend) glEnable(GL_BLEND);
  else glDisable(GL_BLEND);

  glClearColor(state.clearColor.x, state.clearColor.y, state.clearColor.z, state.clearColor.w);
  glBlendFunc(state.blendSourceFactor, state.blendDestinationFactor);
}

RenderState RenderPass::GetCurrentOpenGLRenderState()
{
  RenderState currentState;

  GLboolean depthTestEnabled;
  GLboolean blendEnabled;
  GLint blendSourceFactor;
  GLint blendDestinationFactor;
  Vec4 currentClearColor;

  GLCall(glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled));
  GLCall(glGetBooleanv(GL_BLEND, &blendEnabled));
  GLCall(glGetIntegerv(GL_BLEND_SRC_RGB, &blendSourceFactor));
  GLCall(glGetIntegerv(GL_BLEND_DST_RGB, &blendDestinationFactor));
  GLCall(glGetFloatv(GL_COLOR_CLEAR_VALUE, (float*)(&currentClearColor)));

  return { (bool)depthTestEnabled, (bool)blendEnabled, (GLenum)blendSourceFactor, 
    (GLenum)blendDestinationFactor };
}


Shader& RenderPass::GetShader()
{
  assert(m_shader != nullptr);
  return *m_shader;
}

}
