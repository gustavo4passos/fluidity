#include "filter_pass.hpp"
#include "../utils/glcall.h"
#include "../utils/opengl_utils.hpp"
#include <cassert>

namespace fluidity
{
FilterPass::FilterPass(
    int bufferWidth,
    int bufferHeight,
    const float pointRadius
    )
    : RenderPass(bufferWidth, bufferHeight, 0, pointRadius, 0)
    { /* */ }

bool FilterPass::Init()
{
  m_shader = new Shader("../../shaders/quad_rendering.vert", "../../shaders/filter-narrow-range.frag");
  m_framebuffer.PushAttachment({ GL_R32F, (int)m_bufferWidth, (int)m_bufferHeight, 
      GL_RED, GL_FLOAT
  });

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

  m_framebuffer.Bind();
  m_shader->Bind();
  
  BindTextures();
  GLCall(glBindVertexArray(m_quadVao));
  GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));

  m_shader->Unbind();
  m_framebuffer.Unbind();
}

bool FilterPass::SetUniforms()
{
  m_shader->Bind();
  m_shader->SetUniform1i("u_DoFilter1D", 0);
  m_shader->SetUniform1i("u_FilterSize", 5);
  m_shader->SetUniform1i("u_ScreenWidth", m_bufferWidth);
  m_shader->SetUniform1i("u_ScreenHeight", m_bufferHeight);
  m_shader->SetUniform1i("u_MaxFilterSize", 100);
  m_shader->SetUniform1f("u_ParticleRadius", m_pointRadius);
  m_shader->Unbind();

  return true;
}

void FilterPass::SetInputTexture(GLuint texture, int slot)
{
  m_textureBinds[slot] = texture;
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
