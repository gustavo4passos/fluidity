#pragma once
#include "render_pass.hpp"
#include "framebuffer.hpp"
#include "renderer.h"
#include "shader.h"

namespace fluidity
{
class ParticlePass : public RenderPass
{
public:
    ParticlePass(
      int bufferWidth,
      int bufferHeight,
      int numberOfParticles,
      GLuint particlesVAO,
      FramebufferAttachment renderTargetSpecification,
      const std::string& vsFilepath,
      const std::string& fsFilepath
    );

    virtual bool Init() override;
    virtual void Render() override;

private:
    FramebufferAttachment m_renderTargetSpecification;
    std::string m_vsFilePath;
    std::string m_fsFilePath;
};

};
