#include "particle_render_pass.hpp"
#include "../utils/glcall.h"
#include "../utils/logger.h"
#include <assert.h>
#include <glm/gtc/type_ptr.hpp>

namespace fluidity
{
ParticleRenderPass::ParticleRenderPass(
    const unsigned bufferWidth,
    const unsigned bufferHeight,
    const unsigned numberOfParticles,
    const float pointRadius,
    GLuint particlesVAO)
    : RenderPass(bufferWidth, bufferHeight, numberOfParticles, particlesVAO),
    m_pointRadius(pointRadius)
    { /* */ }

    bool ParticleRenderPass::Init()
    {
        m_shader = new Shader("../../shaders/particle.vert", "../../shaders/particle.frag");
        m_framebuffer.PushAttachment({ GL_RGBA32F, GL_RGBA, GL_FLOAT });
        
        if (!RenderPass::Init()) return false;

        return true;
    }

    void ParticleRenderPass::Render()
    {
        assert(m_shader != nullptr);

        // Save OpenGL state before changing it
        RenderState previousRenderState;
        previousRenderState = GetCurrentOpenGLRenderState();

        m_framebuffer.Bind();
        m_shader->Bind();

        ChangeOpenGLRenderState(m_renderState);

        GLCall(glBindVertexArray(m_vao));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        GLCall(glClear(GL_DEPTH_BUFFER_BIT));
        GLCall(glDrawArrays(GL_POINTS, 0, m_numVertices));

        GLCall(glBindVertexArray(0));
        m_shader->Unbind();
        m_framebuffer.Unbind();

        // ChangeOpenGLRenderState(previousRenderState);
    }

    bool ParticleRenderPass::SetUniforms()
    {
      m_shader->Bind();
      m_shader->SetUniform1ui("u_nParticles", m_numVertices);
      m_shader->SetUniform1i("u_ColorMode", COLOR_MODE_RANDOM);
      m_shader->SetUniform1i("u_UseAnisotropyKernel", 0);
      m_shader->SetUniform1f("u_PointRadius", (float)m_pointRadius);
      m_shader->SetUniform1i("u_ScreenWidth", m_bufferWidth);
      m_shader->SetUniform1i("u_ScreenHeight", m_bufferHeight);
      m_shader->Unbind(); 

      return true;
    }
}
