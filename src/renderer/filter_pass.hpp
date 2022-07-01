#pragma once
#include "render_pass.hpp"
#include <string>
#include <unordered_map>

namespace fluidity
{

class FilterPass : public RenderPass
{
public:
    FilterPass(
      int bufferWidth,
      int bufferHeight,
      const float pointRadius,
      const FramebufferAttachment& renderTargetSpecification,
      const std::string& fsFilePath,
      const bool useDoubleBuffer = false
    );

    virtual bool Init() override;
    virtual void Render() override;
    virtual void SetInputTexture(GLuint texture, int slot = 0);

    // Require double buffering
    // Set current render target as the input texture AND swap framebuffer render targets (0 and 1)
    virtual void SwapBuffers(int textureSlot = 0);

protected:
    void InitQuadVao();
    void BindTextures();
    void UnbindTextures();

    std::string m_fsFilePath;
    FramebufferAttachment m_renderTargetSpecification;
    bool m_useDoubleBuffer;

    GLuint m_quadVao;
    GLuint m_quadVbo;

    std::unordered_map<int, GLuint> m_textureBinds;
};

}
