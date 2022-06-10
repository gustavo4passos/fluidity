#pragma once
#include <string>
#include <cnpy.h>
#include <vector>
#include <GL/glew.h>
#include "Vec.hpp"


class Fluid {
public:
    Fluid()
    : m_Count(0) { }
    bool Load(const std::string& folder, const std::string& prefix, int start, int count);

    cnpy::npz_t& GetFrameData(int frame);

    int GetNumberOfParticles(int frame);
    int GetNumberOfFrames() { return m_Count; }
    vec3* GetFramePosData(int frame);
    GLuint GetFrameVao(int frame);
    cnpy::NpyArray& GetFrameVelData(int frame);

private:
    cnpy::NpyArray& GetFramePosArray(int frame);
    bool LoadFrameToVao(int frame);

    int m_Count;
    std::vector<cnpy::npz_t> m_fileData;
    std::vector<GLuint> m_FrameVaos;
};
