#include "particle_pass.hpp"
#include "../utils/glcall.h"
#include "../utils/opengl_utils.hpp"
#include "../Vec.hpp"
#include <cassert>
#include <limits>

namespace fluidity
{
ParticlePass::ParticlePass(
    int bufferWidth,
    int bufferHeight,
    int numberOfParticles,
    const float pointRadius,
    GLuint particlesVAO,
    FramebufferAttachment renderTargetSpecification,
    const std::string& vsFilepath,
    const std::string& fsFilepath)
    : RenderPass(bufferWidth, bufferHeight, numberOfParticles, pointRadius, particlesVAO),
      m_renderTargetSpecification(renderTargetSpecification),
      m_vsFilePath(vsFilepath),
      m_fsFilePath(fsFilepath)
    { /* */ }

bool ParticlePass::Init()
{
  m_shader = new Shader(m_vsFilePath, m_fsFilePath);
  m_framebuffer.PushAttachment({ m_renderTargetSpecification.internalFormat, 
      m_renderTargetSpecification.pixelFormat, m_renderTargetSpecification.dataType
  });

  if (!RenderPass::Init()) return false;

  return true;
}

void ParticlePass::Render()
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

  // Maximum possible distance
  constexpr float minusInfinity = -std::numeric_limits<float>::infinity();
  GLCall(glClearColor(minusInfinity, minusInfinity, minusInfinity, 1.0));
  GLCall(glClear(GL_COLOR_BUFFER_BIT));
  GLCall(glClear(GL_DEPTH_BUFFER_BIT));
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

  // Restore previous clear color
  glClearColor(currentClearColor.x, currentClearColor.y, currentClearColor.z, currentClearColor.w);
}

}
