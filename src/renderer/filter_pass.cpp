#include "filter_pass.hpp"
#include "../utils/glcall.h"
#include "../utils/opengl_utils.hpp"
#include <cassert>

namespace fluidity
{
FilterPass::FilterPass(
    int bufferWidth,
    int bufferHeight,
    const FramebufferAttachment& renderTargetSpecification,
    const std::string& fsFilePath,
    const bool useDoubleBuffer
    )
    : RenderPass(bufferWidth, bufferHeight, 0, 0),
    m_fsFilePath(fsFilePath),
    m_renderTargetSpecification(renderTargetSpecification),
    m_useDoubleBuffer(useDoubleBuffer)
{ 
  m_renderState.useDepthTest = false;
}

bool FilterPass::Init()
{
  m_shader = new Shader("../../shaders/quad_rendering.vert", m_fsFilePath);
  m_framebuffer.PushAttachment({ m_renderTargetSpecification.internalFormat, 
      m_renderTargetSpecification.pixelFormat, m_renderTargetSpecification.dataType
  });

  if (m_useDoubleBuffer) m_framebuffer.DuplicateAttachment(0);
  if (!RenderPass::Init()) return false;

  InitQuadVao();

  return true;
}

void FilterPass::InitQuadVao()
{
  GLfloat quadVertices[] = 
  {
    -1.f,  1.f, 0.f, 1.f,
    1.f,  1.f, 1.f, 1.f,
    -1.f, -1.f, 0.f, 0.f,
    1.f,  1.f, 1.f, 1.f,
    -1.f, -1.f, 0.f, 0.f,
    1.f, -1.f, 1.f, 0.f
  };

  GLCall(glGenBuffers(1, &m_quadVbo));
  GLCall(glGenVertexArrays(1, &m_quadVao));

  GLCall(glBindVertexArray(m_quadVao));
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo));

  GLCall(glBufferData(
        GL_ARRAY_BUFFER, 
        sizeof(quadVertices), 
        quadVertices, 
        GL_STATIC_DRAW));

  GLCall(glVertexAttribPointer(
        0, 
        2, 
        GL_FLOAT, 
        GL_FALSE, 
        sizeof(GLfloat) * 4, 
        (const GLvoid*)0));

  GLCall(glEnableVertexAttribArray(0));

  GLCall(glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(GLfloat) * 4,
        (const GLvoid*)(2 * sizeof(GLfloat))));
  GLCall(glEnableVertexAttribArray(1));

  GLCall(glBindVertexArray(0));
  GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void FilterPass::Render()
{
  // Sanity check
  assert(m_shader != nullptr);

  RenderState previousRenderState = GetCurrentOpenGLRenderState();
  ChangeOpenGLRenderState(m_renderState);

  m_framebuffer.Bind();
  m_shader->Bind();

  BindTextures();
  GLCall(glBindVertexArray(m_quadVao));
  GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));

  m_shader->Unbind();
  m_framebuffer.Unbind();

  // Restore previous render state
  ChangeOpenGLRenderState(previousRenderState);
}

void FilterPass::SetInputTexture(GLuint texture, int slot)
{
  m_textureBinds[slot] = texture;
}

void FilterPass::SwapBuffers(int textureSlot)
{
  assert(m_useDoubleBuffer);

  // Set current render target as input texture
  SetInputTexture(m_framebuffer.GetAttachment(0));

  m_framebuffer.Bind();
  m_framebuffer.SwapRenderTargets();
  // Detach render target 1, since it's being used as an input texture now
  m_framebuffer.DetachRenderTarget(1);
  m_framebuffer.Unbind();
}

void FilterPass::BindTextures()
{
 for (auto& tPair : m_textureBinds)
  {
    GLCall(glActiveTexture(GL_TEXTURE0 + tPair.first));
    GLCall(glBindTexture(GL_TEXTURE_2D, tPair.second));
  }
}

void FilterPass::UnbindTextures()
{
  for (auto& tPair : m_textureBinds)
  {
    GLCall(glActiveTexture(GL_TEXTURE0 + tPair.first));
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
  }
}

}
