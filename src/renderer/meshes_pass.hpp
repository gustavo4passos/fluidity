#pragma once
#include "renderer/render_pass.hpp"
#include "renderer/model.hpp"

namespace fluidity
{

class MeshesPass : public RenderPass
{
public:
    MeshesPass(int bufferWidth,
        int bufferHeight,
        const std::vector<Model>& models = {});

    virtual bool Init() override;
    virtual void Render() override;

    void AddModel(const Model& model) { m_models.push_back(model); };

private:
    std::vector<Model> m_models;
};

}