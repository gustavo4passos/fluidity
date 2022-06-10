#include "Fluid.hpp"
#include <sstream>
#include <iomanip>
#include <iostream>

bool Fluid::Load(const std::string& folder, const std::string& prefix, int start, int count)
{
    for (int i = 0; i < count; i++)
    {
        std::stringstream fName;
        // TODO: Perform error checking
        fName << std::setw(4) << std::setfill('0') << start + i;
        std::string fileName = folder + std::string("/") + prefix + fName.str() + std::string(".npz");
        m_fileData.push_back(
            cnpy::npz_load(
                fileName
            )
        );
        m_Count = i + 1;
        LoadFrameToVao(i);
    }

    return true;
}

cnpy::npz_t& Fluid::GetFrameData(int frame) 
{
    assert(frame < m_Count);
    return m_fileData[frame];
}

GLuint Fluid::GetFrameVao(int frame)
{
    assert(frame < m_Count);
    return m_FrameVaos[frame];
}


bool Fluid::LoadFrameToVao(int frame)
{
    // TODO: Perform error checking
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, GetFramePosArray(frame).num_bytes(), (const void*)GetFramePosData(frame), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void *)0);
    glEnableVertexAttribArray(0);

    m_FrameVaos.insert(m_FrameVaos.begin() + frame, vao);

    return true;
}


vec3* Fluid::GetFramePosData(int frame)
{
    return GetFramePosArray(frame).data<vec3>();
}

cnpy::NpyArray& Fluid::GetFramePosArray(int frame)
{
    return GetFrameData(frame)["pos"];
}

int Fluid::GetNumberOfParticles(int frame)
{
    const int NUM_COMPONENTS = 3;
    const int COMPONENT_SIZE = 4; // Bytes
    return GetFramePosArray(frame).num_bytes() / NUM_COMPONENTS / COMPONENT_SIZE;
}
 