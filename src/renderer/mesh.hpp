#pragma once
#include "vec.hpp"
#include <vector>
#include <GL/glew.h>

#pragma pack(push, 1)
struct Vertex
{
    vec3 position;
    vec3 normal;
};
#pragma pack(pop)

class Mesh
{
public: 
    Mesh(const std::vector<Vertex>& vertices, 
        const std::vector<unsigned int>& indices);

    // Requires an active OpenGL context
    bool Init();
    void CleanUp();
    
    GLuint GetVao() { return m_vao; }
    GLuint GetIbo() { return m_ibo; }

    std::vector<Vertex>& GetVertices()       { return m_vertices; }
    std::vector<unsigned int>& GetIndices()  { return m_indices;  }

private:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;

    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
};