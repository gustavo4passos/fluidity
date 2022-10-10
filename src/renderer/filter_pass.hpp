#pragma once
#include "render_pass.hpp"
#include <string>

namespace fluidity
{

class FilterPass : public RenderPass
{
public:
    FilterPass(
      int bufferWidth,
      int bufferHeight,
      const std::vector<FramebufferAttachment>& attachments,
      const std::string& fsFilePath,
      const bool useDoubleBuffer = false
);

    virtual bool Init() override;
    virtual void Render() override;

    // Require double buffering
    // Set current render target as the input texture AND swap framebuffer render targets (0 and 1)
    virtual void SwapBuffers(int textureSlot = 0);

protected:
    void InitQuadVao();

    std::string m_fsFilePath;
    bool m_useDoubleBuffer;

    GLuint m_quadVao;
    GLuint m_quadVbo;
};

}
