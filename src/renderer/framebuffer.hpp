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
    bool Init();
    GLuint GetAttachment(int attachmentNumber);

  private:
    FramebufferSpecification m_specification;
    GLuint m_fbo;
    std::vector<GLuint> m_attachments;
  };
}
