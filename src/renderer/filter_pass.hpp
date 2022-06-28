#pragma once
#include "render_pass.hpp"
#include <unordered_map>

namespace fluidity
{

class FilterPass : public RenderPass
{
public:
    FilterPass(
      int bufferWidth,
      int bufferHeight,
      const float pointRadius
    );

    virtual bool Init() override;
    virtual void Render() override;
    virtual void SetInputTexture(GLuint texture, int slot = 0);

protected:
    virtual bool SetUniforms() override;
    void InitQuadVao();
    void BindTextures();
    void UnbindTextures();

    GLuint m_quadVao;
    GLuint m_quadVbo;

    std::unordered_map<int, GLuint> m_textureBinds;
};

}
