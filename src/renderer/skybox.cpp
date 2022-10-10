#include "renderer/skybox.hpp"
#include "utils/logger.h"
#include "utils/glcall.h"
#include "utils/opengl_utils.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cassert>
#include <vector>

namespace fluidity
{
Skybox::Skybox(const std::string& folderPath)
    : m_folderPath(folderPath)
{ /* */ }

bool Skybox::Init()
{
    GLCall(glGenTextures(1, &m_id));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, m_id));

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  

    std::vector<std::vector<std::string>> faces =
    {
      { "right",  "posx" },
      { "left",   "negx" },
      { "top",    "posy" },
      { "bottom", "negy" },
      { "front",  "posz" },
      { "back",   "negz" }
    };

    // Load textures
    int width, height, nChannels;

    int i = 0;
    for (const auto& face : faces)
    {
      unsigned char* data = nullptr;
      for (const auto& posFaceName : face)
      {
        data = stbi_load((m_folderPath + "/" + posFaceName + ".jpg").c_str(),
            &width, &height, &nChannels, 0);
        
        // If unable to load, try to load .png
        if (data == nullptr)
        {
            data = stbi_load((m_folderPath + "/" + posFaceName + ".png").c_str(),
                &width, &height, &nChannels, 0);
        } 

        // If can't find .png either, give up
        if (data == nullptr)
        {
          continue;
        }

        GLint internalFormat = nChannels == 3 ? GL_RGB : GL_RGBA;
        GLint format = nChannels == 3 ? GL_RGB : GL_RGBA;
        GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i++, 0, internalFormat, 
            width, height, 0, format, GL_UNSIGNED_BYTE, data));

        // Face found, don't try alternative face names
        break;
      }
      if (data == nullptr)
      {
          std::string faceNames;
          for (const auto& faceName : face) faceNames = faceNames + std::string(",") + faceName;
          LOG_ERROR("Unable to load image: " + m_folderPath + "/[" + faceNames + "][.png/.jpg]");
          // TODO: Clean up before leaving
          return false;
      }
      else free(data);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (const GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    return true;
}

bool Skybox::CleanUp()
{
    // TODO: Should all be unbind before cleaning up? Or should it be the caller's r
    // responsibility?
    glDeleteTextures(1, &m_id);
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
    return true;
}

}
