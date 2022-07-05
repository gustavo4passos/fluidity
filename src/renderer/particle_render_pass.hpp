#pragma once
#include "render_pass.hpp"
#include "framebuffer.hpp"
#include "renderer.h"
#include "shader.h"
#include <glm/glm.hpp>

namespace fluidity
{

class ParticleRenderPass : public RenderPass
{
public:
    ParticleRenderPass(
      const unsigned bufferWidth,
      const unsigned bufferHeight,
      const unsigned numberOfParticles,
      const float pointRadius,
      GLuint particlesVAO
    );

    virtual bool Init() override;
    virtual void Render() override;

private:
    bool SetUniforms() override;
    float m_pointRadius;
    
    enum ColorMode 
    {
      COLOR_MODE_UNIFORM_MATERIAL = 0,
      COLOR_MODE_RANDOM = 1,
      COLOR_MODE_RAMP = 2
    };
};

};
