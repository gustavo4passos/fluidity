#include "particle_pass.hpp"
#include "../utils/glcall.h"
#include "../utils/opengl_utils.hpp"
#include "../vec.hpp"
#include <cassert>
#include <limits>
#include <cmath>

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
{
  // Maximum possible distance
  constexpr float minusInfinity = -std::numeric_limits<float>::infinity();
  m_renderState.clearColor = { minusInfinity, minusInfinity, minusInfinity, 1.0 };
}

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
  RenderState previousRenderState = GetCurrentOpenGLRenderState();

  ChangeOpenGLRenderState(m_renderState);

  m_framebuffer.Bind();
  m_shader->Bind();

  GLCall(glBindVertexArray(m_particlesVAO));
  GLCall(glClear(GL_COLOR_BUFFER_BIT));
  GLCall(glClear(GL_DEPTH_BUFFER_BIT));
  GLCall(glDrawArrays(GL_POINTS, 0, m_numberOfParticles));

  GLCall(glBindVertexArray(0));
  m_shader->Unbind();
  m_framebuffer.Unbind();

  // Restore previous render state
  ChangeOpenGLRenderState(previousRenderState);
}

}
