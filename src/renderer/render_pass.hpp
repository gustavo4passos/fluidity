#pragma once
#include "framebuffer.hpp"
#include "shader.h"
#include "vec.hpp"
#include <unordered_map>

namespace fluidity
{

struct RenderState
{
  bool useDepthTest             = true;
  bool useBlend                 = false;
  GLenum blendSourceFactor      = GL_ONE;
  GLenum blendDestinationFactor = GL_ZERO;
  Vec4 clearColor               = { 0.f, 0.f, 0.f, 1.f };
  bool cullFaceEnabled          = false;
  GLenum cullFaceMode           = GL_BACK;
};

enum class TextureType
{
  Texture2D,
  Cubemap
};

struct TextureBind
{
  GLuint id = 0;
  int slot = 0;
  TextureType type = TextureType::Texture2D;
  
  TextureBind() = default;
  TextureBind(GLuint id)
    : id(id),
    slot(0),
    type(TextureType::Texture2D)
  { /* */ }

  TextureBind(GLuint id, int slot)
    : id(id),
    slot(slot)
  { /* */ }

  TextureBind(GLuint id, int slot, TextureType type)
    : id(id),
    slot(slot),
    type(type)
  { /* */ }
};

class RenderPass
{
public:
  RenderPass(
    int bufferWidth,
    int bufferHeight,
    int numVertices,
    GLuint vao
  );
  virtual ~RenderPass() { };

  virtual bool Init()   = 0;
  virtual void Render() = 0;
  virtual GLuint GetBuffer(int index = 0) { return m_framebuffer.GetAttachment(index); }

  virtual void SetVAO(GLuint vao)                 { m_vao         = vao;       }
  virtual void SetNumVertices(unsigned nVertices) { m_numVertices = nVertices; }
  virtual bool SetUniformBuffer(const std::string& name, GLuint uniformBlockBinding);

  Shader& GetShader();
  Framebuffer& GetFramebuffer() { return m_framebuffer; }

  virtual void SetRenderState(const RenderState& state) { m_renderState = state; };
  virtual RenderState& GetRenderState() { return m_renderState; } 

  virtual void SetInputTexture(const TextureBind& textureBind);
  void BindTextures();
  void UnbindTextures();

protected:
  virtual bool SetUniformBufferForShader(const std::string& name, GLuint uniformBlockBinding, 
      Shader* shader);
  virtual void ChangeOpenGLRenderState(const RenderState& state);
  virtual RenderState GetCurrentOpenGLRenderState();

  virtual bool SetUniforms() { return true; }
  Shader* m_shader; 
  unsigned m_bufferWidth;
  unsigned m_bufferHeight;
  unsigned m_numVertices;

  GLuint m_vao;
  Framebuffer m_framebuffer;
  RenderState m_renderState;

  std::unordered_map<int, TextureBind> m_textureBinds;

};

}
