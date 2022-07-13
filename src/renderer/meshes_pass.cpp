#pragma once
#include "renderer/meshes_pass.hpp"
#include "utils/glcall.h"
#include <cassert>

namespace fluidity
{

MeshesPass::MeshesPass(int bufferWidth,
    int bufferHeight,
    const std::string& vsFilePath,
    const std::string& fsFilePath,
    const std::vector<FramebufferAttachment> attachments,
    const std::vector<Model>& models)
    : RenderPass(bufferWidth, bufferHeight, 0, 0),
    m_models(models),
    m_vsFilePath(vsFilePath),
    m_fsFilePath(fsFilePath)
{
    for (const auto& attachment : attachments) m_framebuffer.PushAttachment(attachment);
}

bool MeshesPass::Init()
{
    m_shader = new Shader(m_vsFilePath, m_fsFilePath);
    return RenderPass::Init();
}

void MeshesPass::Render()
{
    assert(m_shader != nullptr);
    int viewportState[4];
    glGetIntegerv(GL_VIEWPORT, viewportState);

    // Save OpenGL state before changing it
    RenderState previousRenderState;
    previousRenderState = GetCurrentOpenGLRenderState();

    m_framebuffer.Bind();
    m_shader->Bind();
    ChangeOpenGLRenderState(m_renderState);
    glViewport(0, 0, m_bufferWidth, m_bufferHeight);
    BindTextures();

    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    float minusInfinity = -1000000.f;
    GLCall(glClearBufferfv(GL_COLOR, 1, &minusInfinity));

    for (auto& model : m_models)
    {
        for (auto& mesh : model.GetMeshes())
        {
            GLCall(glBindVertexArray(mesh.GetVao()));
            GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.GetIbo()));
            GLCall(glDrawElements(GL_TRIANGLES, mesh.GetIndices().size(), GL_UNSIGNED_INT, 
                (const void*)0));
        }
    }

    // std::vector<float> p;
    // p.resize(m_bufferWidth * m_bufferHeight);
    // glBindTexture(GL_TEXTURE_2D, m_framebuffer.GetAttachment(1));
    // GLCall(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &p[0]));
    // int i = 0;
    // for(const auto& pixel : p)
    // {
    //     i++;
    //     if (i == 50) break;
    //     if (pixel == -1000000) continue;
    //     std::cout << pixel << " ";
    // }
    // std::cout << "\n\n\n";
    // std::cin.get();
    // std::cin.ignore();
    UnbindTextures();
    GLCall(glBindVertexArray(0));
    m_shader->Unbind();
    m_framebuffer.Unbind();

    glViewport(viewportState[0], viewportState[1], viewportState[2], viewportState[3]);
    ChangeOpenGLRenderState(previousRenderState);

}

}