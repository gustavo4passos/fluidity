#pragma once
#include <string>
#include <cnpy.h>
#include <vector>
#include <GL/glew.h>
#include "vec.hpp"


class Fluid {
public:
    Fluid()
    : m_Count(0) { }
    bool Load(const std::string& folder, const std::string& prefix, int start, int count);
    bool Load(const std::vector<std::string>& npzFileList);

    void CleanUp();

    cnpy::npz_t& GetFrameData(int frame);

    int GetNumberOfParticles(int frame);
    int GetNumberOfFrames() const { return m_Count; }
    const std::vector<std::string>& GetFileList() const { return m_npzFileList; }
    vec3* GetFramePosData(int frame);
    GLuint GetFrameVao(int frame);
    cnpy::NpyArray& GetFrameVelData(int frame);

private:
    cnpy::NpyArray& GetFramePosArray(int frame);
    bool LoadFrameToVao(int frame);

    bool Load();
    int m_Count;
    std::vector<std::string> m_npzFileList;
    std::vector<cnpy::npz_t> m_fileData;
    std::vector<GLuint> m_FrameVaos;
    std::vector<GLuint> m_FrameVbos;
};
