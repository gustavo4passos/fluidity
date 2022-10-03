#pragma once
#include <string>
#include <cnpy.h>
#include <vector>
#include <GL/glew.h>
#include "vec.hpp"

struct FrameData
{
    int nParticles;
    GLuint vao;
    GLuint vbo;
};

class Fluid {
public:
    Fluid() = default;

    bool Load(const std::string& folder, const std::string& prefix, int start, int count);
    bool Load(const std::vector<std::string>& npzFileList);

    void CleanUp();

    int GetNumberOfParticles(int frame);
    int GetNumberOfFrames() const { return m_frameData.size(); }

    const std::vector<std::string>& GetFileList() const { return m_npzFileList; }

    GLuint GetFrameVao(int frame);

private:
    int CalcNumberOfParticles(const cnpy::NpyArray& particleData);
    cnpy::NpyArray& GetFramePosArray(cnpy::npz_t& particleData);
    bool LoadFrameToVao(int frame);
    std::tuple<GLuint, GLuint> LoadParticleDataToVao(const cnpy::NpyArray& data);

    GLenum GetDataTypeFromWordSize(size_t wordSize);

    bool Load();
    std::vector<std::string> m_npzFileList;
    std::vector<FrameData> m_frameData;
};