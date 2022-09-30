#include "framebuffer.hpp"
#include "../utils/glcall.h"
#include "../utils/logger.h"
#include <assert.h>

namespace fluidity
{

Framebuffer::Framebuffer(const FramebufferSpecification& specification)
  : m_specification(specification)
{ /* */ }

bool Framebuffer::Init()
{
  glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
  glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
  glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
  // TODO: Invalidate framebuffer before initializing it, in case it is called
  // more than once (when an attachment is added after it has already been initalized,
  // for example)
  GLCall(glGenFramebuffers(1, &m_fbo));
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

  std::vector<GLenum> attachments;

  for (const auto& a : m_specification.attachments)
  {
    InitAttachment(a);
    int currentAttachment = attachments.size();
    attachments.push_back((GLenum)(GL_COLOR_ATTACHMENT0 + currentAttachment));
  }

  if (m_specification.createDepthBuffer) 
  {
    AttachDepthBuffer();
  }

  // TODO: Clean up in case of failure
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
      LOG_ERROR("Framebuffer is not complete.");
      return false;
  }
  
  // TODO: What if attachments is empty?
  GLCall(glDrawBuffers(attachments.size(), &attachments[0]));
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

  return true;
}

void Framebuffer::PushAttachment(const FramebufferAttachment& attachment)
{ 
  m_specification.attachments.push_back(attachment);
}

void Framebuffer::DuplicateAttachment(int attachment)
{ 
  assert(attachment < m_specification.attachments.size());

  const auto& entry = m_specification.attachments[attachment];
  m_specification.attachments.push_back(entry);
}

void Framebuffer::InitAttachment(const FramebufferAttachment& attachment)
{
    int currentAttachmentIndex = m_attachments.size();
    GLuint texture;
    GLCall(glGenTextures(1, &texture));
    GLCall(glBindTexture(GL_TEXTURE_2D, texture));
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, attachment.internalFormat, m_specification.width, 
          m_specification.height, 0, attachment.pixelFormat, attachment.dataType, nullptr));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + currentAttachmentIndex,
          GL_TEXTURE_2D, texture, 0));

    m_attachments.push_back(texture);
}

void Framebuffer::AttachDepthBuffer()
{
  GLCall(glGenRenderbuffers(1, &m_depthAttachment));
  GLCall(glBindRenderbuffer(GL_RENDERBUFFER, m_depthAttachment));
  GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 
        m_specification.width, m_specification.height));
  GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 
    m_depthAttachment));
  GLCall(glBindRenderbuffer(GL_RENDERBUFFER, 0));
}

void Framebuffer::Bind()
{
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
}

void Framebuffer::Unbind()
{
  GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

GLuint Framebuffer::GetAttachment(int attachmentNumber)
{
  assert(attachmentNumber < m_attachments.size());
  return m_attachments[attachmentNumber];
}

bool Framebuffer::AttachRenderTarget(int attachment)
{
  if(!IsAttachmentValid(attachment))
  {
    LOG_ERROR("Unable to attach render target. Buffer attachment is invalid.");
    return false;
  }

  GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment,
        GL_TEXTURE_2D, m_attachments[attachment], 0));

  return true;
}

bool Framebuffer::DetachRenderTarget(int attachment)
{
  if(!IsAttachmentValid(attachment))
  {
    LOG_ERROR("Unable to detach render target. Buffer attachment is invalid.");
    return false;
  }

  GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment,
        GL_TEXTURE_2D, 0, 0));

  return true;
}

bool Framebuffer::SwapRenderTargets(int buffer0, int buffer1)
{
  if(!IsAttachmentValid(buffer0) || !IsAttachmentValid(buffer1))
  {
    LOG_ERROR("Unable to swap buffer. Buffer number is invalid.");
    return false;
  }

  GLuint buffer0tex = m_attachments[buffer0];
  GLuint buffer1tex = m_attachments[buffer1];
  m_attachments[buffer0] = buffer1tex;
  m_attachments[buffer1] = buffer0tex;

  GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + buffer0,
        GL_TEXTURE_2D, buffer1tex, 0));
  GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + buffer1,
        GL_TEXTURE_2D, buffer0tex, 0));

  return true;
}

bool Framebuffer::IsAttachmentValid(int attachment)
{
  return attachment >= 0 && attachment < m_attachments.size();
}

}
