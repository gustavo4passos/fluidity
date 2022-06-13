#include "depth_pass.hpp"
#include "../utils/glcall.h"
#include "../Vec.hpp"
#include <cassert>
#include <limits>

namespace fluidity
{
DepthPass::DepthPass(
    const unsigned bufferWidth,
    const unsigned bufferHeight,
    const unsigned numberOfParticles,
    const float pointRadius,
    GLuint particlesVAO)
    : RenderPass(bufferWidth, bufferHeight, numberOfParticles, pointRadius, particlesVAO)
    { /* */ }

bool DepthPass::Init()
{
  m_shader = new Shader("../../shaders/depth-pass.vert", "../../shaders/depth-pass.frag");
  m_framebuffer.PushAttachment({ GL_RED, (int)m_bufferWidth, (int)m_bufferHeight, 
      GL_RED, GL_FLOAT
  });

  if (!RenderPass::Init()) return false;

  return true;
}

void DepthPass::Render()
{
  assert(m_shader != nullptr);
  static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 required");

  // Save OpenGL state before changing it
  GLboolean isBlendEnabled;
  GLboolean isDepthTestEnabled;
  Vec4 currentClearColor;

  GLCall(glGetBooleanv(GL_BLEND, &isBlendEnabled));
  GLCall(glGetBooleanv(GL_DEPTH_TEST, &isDepthTestEnabled));
  GLCall(glGetFloatv(GL_COLOR_CLEAR_VALUE, (float*)(&currentClearColor)));

  m_framebuffer.Bind();
  m_shader->Bind();

  GLCall(glBindVertexArray(m_particlesVAO));
  // glBindTexture(GL_TEXTURE_2D, m_buffer);

  GLCall(glEnable(GL_DEPTH_TEST));

  constexpr float minusInfinity = -std::numeric_limits<float>::infinity();
  GLCall(glClearColor(minusInfinity, minusInfinity, minusInfinity, 1.0));
  GLCall(glClear(GL_COLOR_BUFFER_BIT));
  GLCall(glDrawArrays(GL_POINTS, 0, m_numberOfParticles));

  GLCall(glBindVertexArray(0));
  m_shader->Unbind();
  m_framebuffer.Unbind();

  if (!isBlendEnabled)
  {
    GLCall(glDisable(GL_BLEND));
  }
  if (!isDepthTestEnabled)
  {
    GLCall(glDisable(GL_DEPTH_TEST));
  }

  glClearColor(currentClearColor.x, currentClearColor.y, currentClearColor.z, currentClearColor.w);
}

bool DepthPass::SetUniforms()
{
  m_shader->Bind();
  m_shader->SetUniform1i("u_UseAnisotropyKernel", 0);
  m_shader->SetUniform1f("u_PointRadius", (float)m_pointRadius);
  m_shader->SetUniform1i("u_ScreenWidth", m_bufferWidth);
  m_shader->SetUniform1i("u_ScreenHeight", m_bufferHeight);
  m_shader->Unbind(); 

  return true;
}
}
