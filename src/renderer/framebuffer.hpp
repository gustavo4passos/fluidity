#pragma once
#include <GL/glew.h>
#include <vector>

namespace fluidity
{
  struct FramebufferAttachment
  {
    GLint   internalFormat;
    GLsizei width;
    GLsizei height;
    GLenum  pixelFormat;
    GLenum  dataType;
  };

  struct FramebufferSpecification
  {
    std::vector<FramebufferAttachment> attachments;
    // If depthBufferWidth or depthBufferHeight is 0, a depth buffer is not attached
    int depthBufferWidth  = 0; 
    int depthBufferHeight = 0;
  };

  class Framebuffer
  {
  public:
    Framebuffer(const FramebufferSpecification& specification);
    void PushAttachment(const FramebufferAttachment& attachment);
    bool Init();
    void Bind();
    void Unbind();

    GLuint GetAttachment(int attachmentNumber);

  private:
    void InitAttachment(const FramebufferAttachment& attachment);
    void AttachDepthBuffer();

    FramebufferSpecification  m_specification;
    GLuint m_fbo;
    GLuint m_depthAttachment;
    std::vector<GLuint> m_attachments;
  };
}
