#pragma once
#include <GL/glew.h>
#include <vector>

namespace fluidity
{
  struct FramebufferAttachment
  {
    GLint   internalFormat;
    GLenum  pixelFormat;
    GLenum  dataType;
  };

  struct FramebufferSpecification
  {
    std::vector<FramebufferAttachment> attachments;
    GLsizei width;
    GLsizei height;
    bool createDepthBuffer = true;
  };

  class Framebuffer
  {
  public:
    Framebuffer(const FramebufferSpecification& specification);
    void PushAttachment(const FramebufferAttachment& attachment);
    void DuplicateAttachment(int attachment);
    bool Init();
    void Bind();
    void Unbind();

    GLuint GetAttachment(int attachmentNumber);
    // By default, swap targets 0 and 1
    // It will also attach the buffers
    // TODO: It shouldn't attach the buffer, this behavior is not obvious from the caller
    bool SwapRenderTargets(int buffer0 = 0, int buffer1 = 1);

    // Advanced: Misuse will break the framebuffer
    bool AttachRenderTarget(int attachment);
    bool DetachRenderTarget(int attachment);

  private:
    void InitAttachment(const FramebufferAttachment& attachment);
    void AttachDepthBuffer();
    bool IsAttachmentValid(int attachment);

    FramebufferSpecification  m_specification;
    GLuint m_fbo;
    GLuint m_depthAttachment;
    std::vector<GLuint> m_attachments;
  };
}
