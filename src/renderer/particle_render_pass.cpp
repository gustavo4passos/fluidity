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
    : m_bufferWidth(bufferWidth),
    m_bufferHeight(bufferHeight),
    m_numberOfParticles(numberOfParticles),
    m_pointRadius(pointRadius),
    m_particlesVAO(particlesVAO),
    m_particleRendererShader(nullptr)
    { /* */ }

    auto ParticleRenderPass::Init() -> bool
    {
        m_particleRendererShader = new Shader("../../shaders/particle.vert", "../../shaders/particle.frag");

        GLCall(glGenFramebuffers(1, &m_fbo));
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

        GLCall(glGenTextures(1, &m_buffer));
        GLCall(glBindTexture(GL_TEXTURE_2D, m_buffer));
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_bufferWidth, m_bufferHeight, 0, GL_RGBA, GL_FLOAT, nullptr));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_buffer, 0));

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_ERROR("Framebuffer is not complete.");
            return false;
        }
        
        unsigned attachments[1] = { GL_COLOR_ATTACHMENT0 };
        GLCall(glDrawBuffers(1, attachments));
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        SetUniforms();
        return true;
    }

    auto ParticleRenderPass::Render() -> void
    {
        assert(m_particleRendererShader != nullptr);

        GLboolean isBlendEnabled;
        GLboolean isDepthTestEnabled;
        GLCall(glGetBooleanv(GL_BLEND, &isBlendEnabled));
        GLCall(glGetBooleanv(GL_DEPTH_TEST, &isDepthTestEnabled));

        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
        m_particleRendererShader->Bind();
        // m_bilateralFilter->SetUniform1i("kernelRadius", m_kernelRadius);

        GLCall(glBindVertexArray(m_particlesVAO));
        GLCall(glActiveTexture(GL_TEXTURE0));
        glBindTexture(GL_TEXTURE_2D, m_buffer);

        GLCall(glEnable(GL_BLEND));
        GLCall(glEnable(GL_DEPTH_TEST));
        GLCall(glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D,
                m_buffer,
                0));


        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        GLCall(glDrawArrays(GL_POINTS, 0, m_numberOfParticles));

        glBindTexture(GL_TEXTURE_2D, 0);
        GLCall(glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            0,
            0));

        GLCall(glBindTexture(GL_TEXTURE_2D, 0));
        GLCall(glBindVertexArray(0));
        m_particleRendererShader->Unbind();
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        if (!isBlendEnabled)
        {
          GLCall(glDisable(GL_BLEND));
        }
        if (!isDepthTestEnabled)
        {
          GLCall(glDisable(GL_DEPTH_TEST));
        }
    }

    auto ParticleRenderPass::SetTransformationMatrices(
        const glm::mat4 &projectionMatrix,
        const glm::mat4 &view) -> void
    {
    }

    auto ParticleRenderPass::SetUniforms() -> void
    {
      m_particleRendererShader->Bind();
      m_particleRendererShader->SetUniform1ui("u_nParticles", m_numberOfParticles);
      m_particleRendererShader->SetUniform1i("u_ColorMode", COLOR_MODE_RANDOM);
      m_particleRendererShader->SetUniform1i("u_UseAnisotropyKernel", 0);
      m_particleRendererShader->SetUniform1f("u_PointRadius", (float)m_pointRadius);
      m_particleRendererShader->SetUniform1i("u_ScreenWidth", m_bufferWidth);
      m_particleRendererShader->SetUniform1i("u_ScreenHeight", m_bufferHeight);
      m_particleRendererShader->Unbind(); 
    }

    auto ParticleRenderPass::SetUniformBuffer(const std::string& name, 
        GLuint uniformBlockBinding) -> bool
    {
      GLuint index = glGetUniformBlockIndex(m_particleRendererShader->programID(), name.c_str());
      if (index == GL_INVALID_INDEX) return false;

      GLCall(glUniformBlockBinding(m_particleRendererShader->programID(), index, 
            uniformBlockBinding));
      
      return true;
    }
}
