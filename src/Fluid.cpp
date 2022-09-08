#include "Fluid.hpp"
#include "utils/glcall.h"
#include <sstream>
#include <iomanip>
#include <iostream>

bool Fluid::Load(const std::string& folder, const std::string& prefix, int start, int count)
{
    for (int i = 0; i < count; i++)
    {
        std::stringstream fName;
        fName << std::setw(4) << std::setfill('0') << start + i;
        std::string fileName = folder + std::string("/") + prefix + fName.str() + std::string(".npz");
        m_npzFileList.push_back(fileName);
    }

    return Load();
}

bool Fluid::Load(const std::vector<std::string>& npzFileList)
{
    m_npzFileList = npzFileList;
    return Load();
}

bool Fluid::Load()
{
    CleanUp();
    // TODO: Perform error checking
    for (int i = 0; i < m_npzFileList.size(); i++)
    {
        m_fileData.push_back(
            cnpy::npz_load(
                m_npzFileList[i]
            )
        );
        m_Count = i + 1;
        LoadFrameToVao(i);
    }

    return true; 
}


void Fluid::CleanUp()
{
    m_Count = 0;

    for (int i = 0; i < m_FrameVaos.size(); i++)
    {
        GLCall(glDeleteBuffers(1, &m_FrameVbos[i]));
        GLCall(glDeleteVertexArrays(1, &m_FrameVaos[i]));
    }

    m_FrameVaos.clear();
    m_FrameVbos.clear();
    m_fileData.clear();
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

// TODO: This needs to be optimized
bool Fluid::LoadFrameToVao(int frame)
{
    size_t posComponentWordSize = GetFramePosArray(frame).word_size;
    // Only 32 and 64 bit floating-point types are allowed
    assert(posComponentWordSize == 4 || posComponentWordSize == 8);
    
    // TODO: Perform error checking
    GLuint vao, vbo;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glGenBuffers(1, &vbo));
    GLCall(glBindVertexArray(vao));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vbo));

    const void* posArrayPointer;
    if (posComponentWordSize == 4) posArrayPointer = (const void*)GetFramePosData<vec3>(frame);
    else posArrayPointer = (const void*)GetFramePosData<dVec3>(frame);

    GLCall(glBufferData(GL_ARRAY_BUFFER, GetFramePosArray(frame).num_bytes(), posArrayPointer, GL_STATIC_DRAW));
    GLCall(glVertexAttribPointer(0, 3, GetFrameDataType(frame), GL_FALSE, 
        3 * GetFramePosArray(frame).word_size, (const void *)0));
    GLCall(glEnableVertexAttribArray(0));

    m_FrameVaos.insert(m_FrameVaos.begin() + frame, vao);
    m_FrameVbos.insert(m_FrameVbos.begin() + frame, vbo);

    return true;
}

GLenum Fluid::GetFrameDataType(int frame)
{
    auto wordSize = GetFramePosArray(frame).word_size;
    // Only 32 and 64 bit floating-point types are allowed
    assert(wordSize == 4 || wordSize == 8);

    if (wordSize == 4) return GL_FLOAT;
    return GL_DOUBLE;
}

cnpy::NpyArray& Fluid::GetFramePosArray(int frame)
{
    return GetFrameData(frame)["pos"];
}

int Fluid::GetNumberOfParticles(int frame)
{
    if (m_Count == 0) return 0;
    
    const int NUM_COMPONENTS = 3;
    const int COMPONENT_SIZE = GetFramePosArray(frame).word_size; // Bytes
    return GetFramePosArray(frame).num_bytes() / NUM_COMPONENTS / COMPONENT_SIZE;
}
 