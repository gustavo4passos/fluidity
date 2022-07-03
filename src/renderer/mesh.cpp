#include "renderer/mesh.hpp"
#include "utils/glcall.h"
#include <iostream>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : m_vertices(vertices),
    m_indices(indices)
{ /* */ }
    
bool Mesh::Init()
{
    GLCall(glGenVertexArrays(1, &m_vao));
    GLCall(glGenBuffers(1, &m_vbo));
    GLCall(glGenBuffers(1, &m_ibo));

    GLCall(glBindVertexArray(m_vao));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertices.size(), 
        &m_vertices[0], GL_STATIC_DRAW));
    
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), 
        &m_indices[0], GL_STATIC_DRAW));

    GLCall(glEnableVertexAttribArray(0)); // Position
    GLCall(glEnableVertexAttribArray(1)); // Normal
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0));
    GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3)));

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GLCall(glBindVertexArray(0));

    return true;
}