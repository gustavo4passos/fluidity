#pragma once

#include <string>
#include <iostream>

#include <GL/glew.h>
#include <GL/gl.h>

struct ShaderSource {
	std::string vertexShaderSource;
	std::string fragmentShaderSource;
};

class Shader {
public:
	Shader(const std::string& vsFilepath, const std::string& fsFilepath);
	~Shader();

	unsigned int programID() const { return (unsigned int)_programID; }

	void Bind();
	void Unbind();

	// Set uniforms
	// Silent fail means no logging on failure to find uniform name
	void SetUniform1i     (const char* name, GLint v0, bool silentFail = false									    );
	void SetUniform1ui    (const char* name, GLuint v0, bool silentFail = false									    );
	void SetUniform1f     (const char* name, GLfloat v0, bool silentFail = false								    );
	void SetUniform2f     (const char* name, GLfloat v0, GLfloat v1, bool silentFail = false					    );
	void SetUniform3f     (const char* name, GLfloat v0, GLfloat v1, GLfloat v2, bool silentFail = false		    );
	void SetUniform4f     (const char* name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3, bool silentFail = false);
	void SetUniform3fv    (const char* name, GLfloat* v0, bool silentFail = false								    );
	void SetUniformMat4   (const char* name, const void* data, bool silentFail = false							    );
	bool SetUniformBuffer (const char* name, GLuint blockBinding, bool silentFail = false  	 				        );

	// Only for debug purposes
	void PrintActiveAttributes() const;
	void PrintActiveUniforms() const;

	int ID() { return _programID; }

private:
	GLuint _programID;
	ShaderSource _shaderSource;

	std::string _vertexShaderFilepath;
	std::string _fragmentShaderFilepath;

	ShaderSource ParseShader(const std::string& vsFilepath, const std::string& fsFilepath);
	GLuint CreateShader(const ShaderSource& shaderSource);
	GLuint CompileShader(GLenum shaderType, const std::string& source);
	GLint GetUniformLocation(const std::string& name, bool silentFail);

};
