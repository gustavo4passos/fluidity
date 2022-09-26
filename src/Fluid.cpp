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
        cnpy::npz_t particleData = cnpy::npz_load(m_npzFileList[i]);
        auto posParticleData = GetFramePosArray(particleData);
        auto [ frameVao, frameVbo ] = LoadParticleDataToVao(posParticleData);
        m_frameData.push_back({ CalcNumberOfParticles(posParticleData), frameVao, frameVbo });
    }

    return true; 
}


void Fluid::CleanUp()
{
    for (auto& f : m_frameData)
    {
        GLCall(glDeleteBuffers(1, &f.vbo));
        GLCall(glDeleteVertexArrays(1, &f.vao));
    }

    m_frameData.clear();
}

GLuint Fluid::GetFrameVao(int frame)
{
    assert(frame < GetNumberOfFrames());
    return m_frameData[frame].vao;
}

// TODO: Error checking. glBufferData and glGen[VertexArrays, Buffers] might throw a memory
// error. How to deal with datasets that can't fit in memory? Initially, maybe just unload
// them, but a data streaming option might be interesting for these cases.
// TODO: [Optimization] This needs to be optimized
std::tuple<GLuint, GLuint> Fluid::LoadParticleDataToVao(const cnpy::NpyArray& data)
{
    size_t posComponentWordSize = data.word_size;
    // Only 32 and 64 bit floating-point types are allowed
    assert(posComponentWordSize == 4 || posComponentWordSize == 8);
    
    // TODO: Perform error checking
    GLuint vao, vbo;
    // TODO: [Optimization] A VAO for each particle is probably unecessary. The layout is
    // the same for every 32 OR 64 fp particle data. (So, likely, only 2 vaos are necessary)
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glGenBuffers(1, &vbo));
    GLCall(glBindVertexArray(vao));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vbo));

    const void* posArrayPointer;
    if (posComponentWordSize == 4) posArrayPointer = (const void*)data.data<vec3>();
    else posArrayPointer = (const void*)data.data<dVec3>();

    GLCall(glBufferData(GL_ARRAY_BUFFER, data.num_bytes(), posArrayPointer, GL_STATIC_DRAW));
    GLCall(glVertexAttribPointer(0, 3, GetDataTypeFromWordSize(posComponentWordSize), 
        GL_FALSE, 3 * posComponentWordSize, (const void *)0));
    GLCall(glEnableVertexAttribArray(0));

    return { vao, vbo };
}

GLenum Fluid::GetDataTypeFromWordSize(size_t wordSize)
{
    // Only 32 and 64 bit floating-point types are allowed
    assert(wordSize == 4 || wordSize == 8);

    if (wordSize == 4) return GL_FLOAT;
    return GL_DOUBLE;
}

cnpy::NpyArray& Fluid::GetFramePosArray(cnpy::npz_t& particleData)
{
    return particleData["pos"];
}

int Fluid::GetNumberOfParticles(int frame)
{
    assert(frame < GetNumberOfFrames());
    return m_frameData[frame].nParticles;
}

int Fluid::CalcNumberOfParticles(const cnpy::NpyArray& particleData)
{
    const int NUM_COMPONENTS = 3;
    const int COMPONENT_SIZE = particleData.word_size; // Bytes
    return particleData.num_bytes() / NUM_COMPONENTS / COMPONENT_SIZE;
}
 