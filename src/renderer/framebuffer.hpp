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

    FramebufferSpecification m_specification;
    GLuint m_fbo;
    std::vector<GLuint> m_attachments;
  };
}
