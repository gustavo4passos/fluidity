#include "renderer/mesh.hpp"
#include <iostream>

Mesh::Mesh(const std::vector<vec3>& vertices, const std::vector<unsigned>& indices)
    : m_vertices(vertices),
    m_indices(indices)
{ /* */  }
    
bool Mesh::Init()
{
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ibo);
    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * m_vertices.size(), 
        &m_vertices[0], GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned), 
        &m_indices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}