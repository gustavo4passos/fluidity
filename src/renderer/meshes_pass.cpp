#pragma once
#include "renderer/meshes_pass.hpp"
#include "utils/glcall.h"
#include <cassert>

namespace fluidity
{

MeshesPass::MeshesPass(int bufferWidth,
    int bufferHeight,
    const std::vector<Model>& models)
    : RenderPass(bufferWidth, bufferHeight, 0, 0),
    m_models(models)
{ /* */ }

bool MeshesPass::Init()
{
    m_shader = new Shader("../../shaders/mesh.vert", "../../shaders/mesh.frag");
    m_framebuffer.PushAttachment({ GL_RGB32F, GL_RGB, GL_FLOAT });
    m_framebuffer.PushAttachment({ GL_R32F,   GL_RED, GL_FLOAT });

    return RenderPass::Init();
}

void MeshesPass::Render()
{
    assert(m_shader != nullptr);
    // Save OpenGL state before changing it
    RenderState previousRenderState;
    previousRenderState = GetCurrentOpenGLRenderState();

    m_framebuffer.Bind();
    m_shader->Bind();

    ChangeOpenGLRenderState(m_renderState);

    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    float minusInfinity = -1000000.f;
    glClearBufferfv(GL_COLOR, 1, &minusInfinity);

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

    GLCall(glBindVertexArray(0));
    m_shader->Unbind();
    m_framebuffer.Unbind();

    ChangeOpenGLRenderState(previousRenderState);

}

}