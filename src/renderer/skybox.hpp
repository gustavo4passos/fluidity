#pragma once
#include <string>
#include <GL/glew.h>

namespace fluidity
{

class Skybox
{
public:
    Skybox() = default;
    Skybox(const std::string& folderPath);
    void SetFolderPath(const std::string& folderPath) { m_folderPath = folderPath; };
    bool Init();
    bool CleanUp();

    GLuint GetTextureID() { return m_id; }
    GLuint GetVao() { return m_vao; }

private:
    std::string m_folderPath;
    GLuint m_id;
    GLuint m_vao;
    GLuint m_vbo;
};

}