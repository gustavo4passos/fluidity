#include "shader.h"

#include <fstream>
#include <sstream>
#include <GL/glu.h>

#include "../utils/logger.h"
#include "../utils/glcall.h"

Shader::Shader(const ShaderPaths& shaderPaths)
  : m_shaderPaths(shaderPaths)
{
  m_shaderSource = ParseShader(shaderPaths);
  m_programID    = CreateShader(m_shaderSource);
}

Shader::~Shader() {
  //Unbind(); 
  //GLCall(glDeleteProgram(m_programID));
}

void Shader::Bind() {
  GLCall(glUseProgram(this->m_programID));
}

void Shader::Unbind() {
  GLCall(glUseProgram(0));
}

void Shader::SetUniform1f(const char* name, GLfloat v0, bool silentFail){
  GLCall(glUniform1f(GetUniformLocation(name, silentFail), v0));
}

void Shader::SetUniform2f(const char* name, GLfloat v0, GLfloat v1, bool silentFail){
  GLCall(glUniform2f(GetUniformLocation(name, silentFail), v0, v1));
}

void Shader::SetUniform3f(const char* name, GLfloat v0, GLfloat v1, GLfloat v2, bool silentFail) {
  GLCall(glUniform3f(GetUniformLocation(name, silentFail), v0, v1, v2));
}

void Shader::SetUniform4f(const char* name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3, bool silentFail){
  GLCall(glUniform4f(GetUniformLocation(name, silentFail), v0, v1, v2, v3));
}

void Shader::SetUniformMat4(const char* name, const void* data, bool silentFail){
  GLCall(glUniformMatrix4fv(GetUniformLocation(name, silentFail), 1, GL_FALSE, (const GLfloat*)data));
}

bool Shader::SetUniformBuffer (const char* name, GLuint blockBinding, bool silentFail)
{
  GLuint index = glGetUniformBlockIndex(m_programID, name);
  if (index == GL_INVALID_INDEX) return false;

  GLCall(glUniformBlockBinding(m_programID, index, 
        blockBinding));
  
  return true;
}

void Shader::SetUniform3fv(const char* name, GLfloat* v0, bool silentFail)
{
  GLCall(glUniform3fv(GetUniformLocation(name, silentFail), 1, v0));
}

void Shader::SetUniform1i(const char* name, GLint v0, bool silentFail)
{
  GLCall(glUniform1i(GetUniformLocation(name, silentFail), v0));
}

void Shader::SetUniform1ui(const char* name, GLuint v0, bool silentFail)
{
  GLCall(glUniform1ui(GetUniformLocation(name, silentFail), v0));
}

void Shader::PrintActiveAttributes() const {
  GLint i;
  GLint count;
  GLint size;
  GLenum type;

  const GLsizei bufSize = 16;
  GLchar name[bufSize];
  GLsizei length;

  GLCall(glGetProgramiv(m_programID, GL_ACTIVE_ATTRIBUTES,  &count));
  std::cout << "There are " << count << " active attributes.\n";

  for(i = 0; i < count; i++) {
    GLCall(glGetActiveAttrib(m_programID, (GLuint)i, bufSize, &length, &size, &type, name));
    std::cout << "Attribute " << i << " type " << type << " name " << name << "\n";
  }
}

void Shader::PrintActiveUniforms() const {
  GLint i;
  GLint count;
  GLint size;
  GLenum type;

  const GLsizei bufSize = 16;
  GLchar name[bufSize];
  GLsizei length;

  GLCall(glGetProgramiv(m_programID, GL_ACTIVE_UNIFORMS,  &count));
  std::cout << "There are " << count << " active uniforms.\n";

  for(i = 0; i < count; i++) {
    GLCall(glGetActiveUniform(m_programID, (GLuint)i, bufSize, &length, &size, &type, name));
    std::cout << "Uniform " << i << " type " << type << " name " << name << std::endl;
  }
}

ShaderSource Shader::ParseShader(const ShaderPaths& shaderPaths) 
{
  std::ifstream vsFile(shaderPaths.vertexShaderPath);
  std::ifstream fsFile(shaderPaths.fragmentShaderPath);
  std::ifstream gsFile(shaderPaths.geometryShaderPath);

  std::stringstream vsSource;
  std::stringstream fsSource;
  std::stringstream gsSource;

  std::string line;

  // Checks if vsFile was opened successfully
  if(vsFile.fail()) {
    LOG_ERROR("Unable to open file " + m_shaderSource.vertexShaderSource);
    DEBUG_BREAK();
  }

  // Checks if fsFile was opened succesfully
  if(fsFile.fail()) {
    LOG_ERROR("Unable to open file " + m_shaderPaths.fragmentShaderPath);
    DEBUG_BREAK();
  }

  // Only try to open geometry shader if it is not empty
  if (m_shaderPaths.HasShader(GL_GEOMETRY_SHADER) && gsFile.fail())
  {
    LOG_ERROR("Unable to open file " + m_shaderPaths.geometryShaderPath);
    DEBUG_BREAK();
  }

  // Reads vertex shader file to string vsSource
  while(getline(vsFile, line)) {
    vsSource << line << '\n';
  }

  // Reads fragment shader file to string fsSource
  while(getline(fsFile, line)) {
    fsSource << line << '\n';
  }

  ShaderSource shaderSource;
  shaderSource.vertexShaderSource = vsSource.str();
  shaderSource.fragmentShaderSource = fsSource.str();
  shaderSource.geometryShaderSource = std::string();  // Empty string, be default

  // Reads geometry shader, if it's available
  if (m_shaderPaths.HasShader(GL_GEOMETRY_SHADER)) {
    while(getline(gsFile, line)) gsSource << line << '\n';
    shaderSource.geometryShaderSource = gsSource.str();
  }

  return shaderSource;
}

GLuint Shader::CreateShader(const ShaderSource& shaderSource) {
  GLuint programID = glCreateProgram();

  if(programID == 0) LOG_ERROR("OpenGL Error: Unable to create program.\n");

  GLuint vs = CompileShader(GL_VERTEX_SHADER, shaderSource.vertexShaderSource);
  GLuint fs = CompileShader(GL_FRAGMENT_SHADER, shaderSource.fragmentShaderSource);
  GLuint gs;

  GLCall(glAttachShader(programID, vs));
  GLCall(glAttachShader(programID, fs));

  if (m_shaderPaths.HasShader(GL_GEOMETRY_SHADER)) {
    gs = CompileShader(GL_GEOMETRY_SHADER, shaderSource.geometryShaderSource);
    GLCall(glAttachShader(programID, gs));
  }

  GLCall(glLinkProgram(programID));

  // Check link status
  GLint linkStatus;
  glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus);
  if (linkStatus != GL_TRUE)
  {
    GLchar infoLog[256];
    glGetProgramInfoLog(programID, 255, nullptr, infoLog);

    LOG_ERROR("Error while linking program with vertex shader: " + m_shaderPaths.vertexShaderPath +
      ": " + infoLog);
  }

  // Shaders are now linked to a program, so we no longer need them
  GLCall(glDeleteShader(vs));
  GLCall(glDeleteShader(fs));
  if (m_shaderPaths.HasShader(GL_GEOMETRY_SHADER)) glDeleteShader(gs);

  return programID;
}

GLuint Shader::CompileShader(GLenum shaderType, const std::string& source) {
  GLuint shader = glCreateShader(shaderType);

  if(shader == 0) LOG_ERROR("OpenGL Error: Unable to create Shader.");

  const char* src = source.c_str();
  GLCall(glShaderSource(shader, 1, &src, nullptr));
  GLCall(glCompileShader(shader));

  GLint compileStatus;
  GLCall(glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus));
  if(compileStatus != GL_TRUE) {
    GLchar log[512];
    GLCall(glGetShaderInfoLog(shader, 512, nullptr, log));
    std::string shaderFilePath;

    switch(shaderType)
    {
      case (GL_VERTEX_SHADER):
      {
        shaderFilePath = m_shaderPaths.vertexShaderPath;
        break;
      }
      case (GL_FRAGMENT_SHADER):
      {
        shaderFilePath = m_shaderPaths.fragmentShaderPath;
        break;
      }
      case (GL_GEOMETRY_SHADER):
      {
        shaderFilePath = m_shaderPaths.geometryShaderPath;
        break;
      }
      default: break;
    }

    LOG_ERROR("Unable to compile shader: file: " + shaderFilePath + std::string(" \n") + std::string(log));
  }

  return shader;
}

GLint Shader::GetUniformLocation(const std::string& name, bool silentFail) {
  GLint location = glGetUniformLocation(this->m_programID, name.c_str());

  // If uniform isn't found in program
  if(location == -1 && !silentFail) LOG_ERROR("in shader " + m_shaderPaths.vertexShaderPath + 
      ", " + m_shaderPaths.geometryShaderPath + ": Unable to find uniform: " + name);

  return location;
}
