#include "framebuffer.hpp"
#include "../utils/GLCall.h"
#include "../utils/logger.h"
#include <assert.h>

namespace fluidity
{

Framebuffer::Framebuffer(const FramebufferSpecification& specification)
  : m_specification(specification)
{ /* */ }

bool Framebuffer::Init()
{
  
  GLCall(glGenFramebuffers(1, &m_fbo));
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

  int currentAttachment = 0;
  std::vector<GLenum> attachments;

  for (const auto& a : m_specification.attachments)
  {
    GLuint texture;
    GLCall(glGenTextures(1, &texture));
    GLCall(glBindTexture(GL_TEXTURE_2D, texture));
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, a.internalFormat, a.width, a.height, 0, 
          a.pixelFormat, a.dataType, nullptr));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + currentAttachment, GL_TEXTURE_2D, texture, 0));

    m_attachments.push_back(texture);
    attachments.push_back((GLenum)(GL_COLOR_ATTACHMENT0 + currentAttachment));
    currentAttachment++;
  }


  // TODO: Clean up in case of failure
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
      LOG_ERROR("Framebuffer is not complete.");
      return false;
  }
  
  GLCall(glDrawBuffers(attachments.size(), &attachments[0]));
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  return true;
}

GLuint Framebuffer::GetAttachment(int attachmentNumber)
{
  assert(attachmentNumber < m_attachments.size());
  return m_attachments[attachmentNumber];
}
}
