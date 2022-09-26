#include "render_pass.hpp"
#include "../utils/glcall.h"
#include <cassert>

namespace fluidity
{
RenderPass::RenderPass(
  int bufferWidth,
  int bufferHeight,
  int numVertices,
  GLuint vao)
  : m_bufferWidth(bufferWidth),
  m_bufferHeight(bufferHeight),
  m_numVertices(numVertices),
  m_vao(vao),
  m_framebuffer({ {}, bufferWidth, bufferHeight }),
  m_renderState({})
{ /* */ } 

bool RenderPass:: Init()
{
    if (!m_framebuffer.Init()) return false;
    if (!SetUniforms())       return false;
    return true;
}

bool RenderPass::SetUniformBuffer(const std::string& name, GLuint uniformBlockBinding)
{
  return m_shader->SetUniformBuffer(name.c_str(), uniformBlockBinding);
}

bool RenderPass::SetUniformBufferForShader(const std::string& name, GLuint uniformBlockBinding, 
  Shader* shader)
{
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

  if (state.cullFaceEnabled) glEnable(GL_CULL_FACE);
  else glDisable(GL_CULL_FACE);
  glCullFace(state.cullFaceMode);
}

RenderState RenderPass::GetCurrentOpenGLRenderState()
{
  RenderState currentState;

  GLboolean depthTestEnabled;
  GLboolean blendEnabled;
  GLint blendSourceFactor;
  GLint blendDestinationFactor;
  Vec4 currentClearColor;
  GLboolean cullFaceEnabled;
  GLint cullFaceMode;

  GLCall(glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled));
  GLCall(glGetBooleanv(GL_BLEND, &blendEnabled));
  GLCall(glGetIntegerv(GL_BLEND_SRC_RGB, &blendSourceFactor));
  GLCall(glGetIntegerv(GL_BLEND_DST_RGB, &blendDestinationFactor));
  GLCall(glGetFloatv(GL_COLOR_CLEAR_VALUE, (float*)(&currentClearColor)));
  GLCall(glGetBooleanv(GL_CULL_FACE, &cullFaceEnabled));
  GLCall(glGetIntegerv(GL_CULL_FACE_MODE, &cullFaceMode));

  return { (bool)depthTestEnabled, (bool)blendEnabled, (GLenum)blendSourceFactor, 
    (GLenum)blendDestinationFactor, currentClearColor, (bool)cullFaceEnabled, (GLenum)cullFaceMode };
}


Shader& RenderPass::GetShader()
{
  assert(m_shader != nullptr);
  return *m_shader;
}

void RenderPass::SetInputTexture(GLuint texture, int slot)
{
  m_textureBinds[slot] = texture;
}

void RenderPass::BindTextures()
{
 for (auto& tPair : m_textureBinds)
  {
    GLCall(glActiveTexture(GL_TEXTURE0 + tPair.first));
    GLCall(glBindTexture(GL_TEXTURE_2D, tPair.second));
  }
}

void RenderPass::UnbindTextures()
{
  for (auto& tPair : m_textureBinds)
  {
    GLCall(glActiveTexture(GL_TEXTURE0 + tPair.first));
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
  }
}

}
