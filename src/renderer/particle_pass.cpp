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
    GLuint particlesVAO,
    FramebufferAttachment renderTargetSpecification,
    const std::string& vsFilepath,
    const std::string& fsFilepath)
    : RenderPass(bufferWidth, bufferHeight, numberOfParticles, particlesVAO),
      m_renderTargetSpecification(renderTargetSpecification),
      m_vsFilePath(vsFilepath),
      m_fsFilePath(fsFilepath)
{
  // Maximum possible distance
  constexpr float minusInfinity = -std::numeric_limits<float>::infinity();
  m_renderState.clearColor = { minusInfinity, minusInfinity, minusInfinity, 1.0 };
  m_framebuffer.PushAttachment(m_renderTargetSpecification);
}

bool ParticlePass::Init()
{
  m_shader = new Shader(m_vsFilePath, m_fsFilePath);
  if (!RenderPass::Init()) return false;

  return true;
}

void ParticlePass::Render()
{
  assert(m_shader != nullptr);
  int viewportState[4];
  glGetIntegerv(GL_VIEWPORT, viewportState);

  // Save OpenGL state before changing it
  RenderState previousRenderState = GetCurrentOpenGLRenderState();
  m_framebuffer.Bind();
  ChangeOpenGLRenderState(m_renderState);
  glViewport(0, 0, m_bufferWidth, m_bufferHeight);

  GLCall(glClear(GL_COLOR_BUFFER_BIT));
  if (m_renderState.useDepthTest) GLCall(glClear(GL_DEPTH_BUFFER_BIT));

  m_shader->Bind();

  GLCall(glBindVertexArray(m_vao));
  GLCall(glDrawArrays(GL_POINTS, 0, m_numVertices));

  GLCall(glBindVertexArray(0));
  m_shader->Unbind();
  m_framebuffer.Unbind();

  // Restore previous render state
  glViewport(viewportState[0], viewportState[1], viewportState[2], viewportState[3]);
  ChangeOpenGLRenderState(previousRenderState);
}

}
