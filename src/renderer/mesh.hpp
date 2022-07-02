#pragma once
#include "vec.hpp"
#include <vector>
#include <GL/glew.h>

class Mesh
{
public: 
    Mesh(const std::vector<vec3>& vertices, 
        const std::vector<unsigned>& indices);

    // Requires an active OpenGL context
    bool Init();
    
    GLuint GetVao() { return m_vao; }
    GLuint GetIbo() { return m_ibo; }

    std::vector<vec3>& GetVertices()     { return m_vertices; }
    std::vector<unsigned>& GetIndices()  { return m_indices;  }

private:
    std::vector<vec3> m_vertices;
    std::vector<unsigned> m_indices;

    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
};