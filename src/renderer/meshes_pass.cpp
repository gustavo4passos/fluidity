#pragma once
#include "renderer/meshes_pass.hpp"
#include "utils/glcall.h"
#include <glm/gtc/type_ptr.hpp>
#include <cassert>

namespace fluidity
{

MeshesPass::MeshesPass(int bufferWidth,
    int bufferHeight,
    const std::string& vsFilePath,
    const std::string& fsFilePath,
    const std::vector<FramebufferAttachment>& attachments,
    Scene* scene)
    : RenderPass(bufferWidth, bufferHeight, 0, 0),
    m_vsFilePath(vsFilePath),
    m_fsFilePath(fsFilePath),
    m_hasSkybox(false),
    m_scene(scene)
{
    for (const auto& attachment : attachments) m_framebuffer.PushAttachment(attachment);
}

bool MeshesPass::Init()
{
    m_shader = new Shader(m_vsFilePath, m_fsFilePath);
    m_skybBoxShader = new Shader("../../shaders/skybox.vert",
    "../../shaders/skybox.frag");

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
    ChangeOpenGLRenderState(m_renderState);
    glViewport(0, 0, m_bufferWidth, m_bufferHeight);

    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    float minusInfinity = -1000000.f;
    GLCall(glClearBufferfv(GL_COLOR, 1, &minusInfinity));

    m_shader->Bind();
    BindTextures();

    // TODO: The per-model uniforms and context changes are acceptable here only because
    // the number of models is admitted to be short
    for (auto& modelPair : m_scene->models)
    {
        auto& model = modelPair.second;
        if (!model.IsVisible()) continue;

        if (model.GetHideFrontFaces()) 
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        }
        else
        {
            glDisable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
        
        const auto& material = model.GetMaterial();
        auto diffuse         = material.diffuse;
        auto specular        = material.specular;
        m_shader->SetUniform1i("uInvertNormals", model.GetHideFrontFaces() ? 1 : 0, true);
        m_shader->SetUniform3f("uDiffuse", diffuse.x, diffuse.y, diffuse.z, true);
        m_shader->SetUniform3f("uSpecular", specular.x, specular.y, specular.z, true);
        m_shader->SetUniform1f("uShininess", material.shininess, true);
        m_shader->SetUniform1i("uEmissive", material.emissive ? 1 : 0, true);
        m_shader->SetUniform1f("uReflectiveness", material.reflectiveness, true);

        m_shader->SetUniformMat4("model", glm::value_ptr(model.GetModelMatrix()), true);

        for (auto& mesh : model.GetMeshes())
        {
            GLCall(glBindVertexArray(mesh.GetVao()));
            GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.GetIbo()));
            GLCall(glDrawElements(GL_TRIANGLES, mesh.GetIndices().size(), GL_UNSIGNED_INT, 
                (const void*)0));
        }
    }

    glDisable(GL_CULL_FACE);
    if (m_hasSkybox)
    {
        RenderSkybox(previousRenderState);
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

void MeshesPass::RenderSkybox(const RenderState& previousRenderState)
{
    // Save current depth func before changing it
    GLint previousDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunc);   
    if (previousDepthFunc != GL_LEQUAL) glDepthFunc(GL_LEQUAL);

    m_skybBoxShader->Bind();
    GLCall(glBindVertexArray(m_skybox.GetVao()));
    GLCall(glActiveTexture(GL_TEXTURE0));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox.GetTextureID()));
    GLCall(glDrawArrays(GL_TRIANGLES, 0, 36));

    if (previousDepthFunc != GL_LEQUAL) glDepthFunc(previousDepthFunc);
}

bool MeshesPass::SetUniformBuffer(const std::string& name, GLuint uniformBlockBinding)
{
    if (!m_shader->SetUniformBuffer(name.c_str(), uniformBlockBinding))        return false;
    if (!m_skybBoxShader->SetUniformBuffer(name.c_str(), uniformBlockBinding)) return false;
    return false;
}

void MeshesPass::AddSkybox(const Skybox& skybox)
{
    if (m_hasSkybox) m_skybox.CleanUp();

    m_hasSkybox = true;
    m_skybox = skybox;
}

void MeshesPass::RemoveSkybox()
{
    if (m_hasSkybox) m_skybox.CleanUp();
    m_hasSkybox = false;
}

Skybox& MeshesPass::GetSkybox()
{
    assert(m_hasSkybox);
    return m_skybox;
}

}
