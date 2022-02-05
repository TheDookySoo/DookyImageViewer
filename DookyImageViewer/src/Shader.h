#ifndef SHADER_H
#define SHADER_H

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

namespace Dooky {
	class Shader {
	private:
		enum class ShaderType {
			NONE = -1,
			VERTEX = 0,
			FRAGMENT = 1
		};

		struct ShaderProgramSource {
			std::string vertexSource;
			std::string fragmentSource;
		};

		unsigned int shaderId;
		bool shaderCreated;

		ShaderProgramSource ParseShader(const std::string& source);
		unsigned int CompileShader(unsigned int type, const std::string& source);
		unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);

		int GetUniformLocation(const char* name);
	public:
		Shader();
		~Shader();

		void Bind();
		void Unbind();

		void LoadShaderFile(const std::string& filePath);
		void LoadShaderFromMemory(const std::string& shader);
		unsigned int GetId();

		// Uniforms
		void SetUniformBool(const char* name, bool boolean); // Exact same as SetUniform1i

		void SetUniform1f(const char* name, float x);
		void SetUniform2f(const char* name, float x, float y);
		void SetUniform3f(const char* name, float x, float y, float z);
		void SetUniform4f(const char* name, float x, float y, float z, float w);

		void SetUniform1i(const char* name, int x);
		void SetUniform2i(const char* name, int x, int y);
		void SetUniform3i(const char* name, int x, int y, int z);
		void SetUniform4i(const char* name, int x, int y, int z, int w);

		void SetUniformMat2fv(const char* name, glm::mat2 data);
		void SetUniformMat3fv(const char* name, glm::mat3 data);
		void SetUniformMat4fv(const char* name, glm::mat4 data);
	};
}

#endif