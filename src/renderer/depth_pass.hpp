#pragma once
#include "render_pass.hpp"
#include "framebuffer.hpp"
#include "renderer.h"
#include "shader.h"

namespace fluidity
{

class DepthPass : public RenderPass
{
public:
    DepthPass(
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
};

};
