#pragma once
#include "framebuffer.hpp"
#include "renderer.h"
#include "shader.h"
#include <glm/glm.hpp>

namespace fluidity
{


class ParticleRenderPass
{
public:
    ParticleRenderPass(
    const unsigned bufferWidth,
    const unsigned bufferHeight,
    const unsigned numberOfParticles,
    const float pointRadius,
    GLuint particlesVAO);

    auto Init() -> bool;
    auto Render() -> void;
    auto GetBuffer() -> GLuint { return m_framebuffer.GetAttachment(0); }

    auto SetParticlesVAO(GLuint particlesVAO) -> void { m_particlesVAO = particlesVAO; }
    auto SetNumberOfParticles(unsigned nParticles) -> void { m_numberOfParticles = nParticles; }
    auto SetUniformBuffer(const std::string& name, GLuint uniformBlockBinding) -> bool;

private:
    auto SetUniforms() -> void;
    unsigned m_bufferWidth;
    unsigned m_bufferHeight;
    unsigned m_numberOfParticles;
    float m_pointRadius;

    GLuint m_buffer;
    GLuint m_fbo;
    GLuint m_particlesVAO;
    
    Shader* m_particleRendererShader; 

    Framebuffer m_framebuffer;

    enum ColorMode 
    {
      COLOR_MODE_UNIFORM_MATERIAL = 0,
      COLOR_MODE_RANDOM = 1,
      COLOR_MODE_RAMP = 2
    };
};

};
