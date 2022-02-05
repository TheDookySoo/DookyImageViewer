#include "Shader.h"

namespace Dooky {
    Shader::ShaderProgramSource Shader::ParseShader(const std::string& source) {
        std::string line;
        std::stringstream ss[2]; // Stores vertex shader and fragment shader
        ShaderType type = ShaderType::NONE;

        std::istringstream stream(source);

        while (std::getline(stream, line)) { // Loop through every line of shader file
            if (line.find("#shader") != std::string::npos) { // Found the shader declaration
                // Check what type of shader this is on the same line, e.g #shader vertex declares a vertex shader

                if (line.find("vertex") != std::string::npos) {
                    type = ShaderType::VERTEX;
                } else if (line.find("fragment") != std::string::npos) {
                    type = ShaderType::FRAGMENT;
                }
            } else {
                ss[(int)type] << line << "\n"; // Use the type of the shader last detected to append shader code
            }
        }

        return { ss[0].str(), ss[1].str() };
    }

    unsigned int Shader::CompileShader(unsigned int type, const std::string& source) {
        unsigned int id = glCreateShader(type);
        const char* src = source.c_str();

        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);

        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);

        // Error handling
        if (result == GL_FALSE) {
            int length;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

            char* message = (char*)_malloca(length * sizeof(char));
            glGetShaderInfoLog(id, length, &length, message);

            std::cout << "SHADER ERROR: Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
            std::cout << "MESSAGE: " << message << std::endl;

            glDeleteShader(id);
            return 0;
        }

        return id;
    }

    unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
        unsigned int program = glCreateProgram();
        unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);     // Vertex Shader
        unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader); // Fragment Shader

        glAttachShader(program, vs);
        glAttachShader(program, fs);

        glLinkProgram(program);
        glValidateProgram(program);

        glDeleteShader(vs);
        glDeleteShader(fs);

        return program;
    }

    Shader::Shader() {
        shaderId = 0;
        shaderCreated = false;
    }

    Shader::~Shader() {
        if (shaderCreated == true) {
            glDeleteProgram(shaderId);
        }
    }

    void Shader::Bind() {
        glUseProgram(shaderId);
    }

    void Shader::Unbind() {
        glUseProgram(0);
    }

    void Shader::LoadShaderFile(const std::string& filePath) {
        if (!std::filesystem::is_regular_file(filePath)) {
            std::cout << "SHADER ERROR: filePath is not a regular file or doesn't exist." << std::endl;
            return;
        }

        // Read all
        std::ifstream stream(filePath);
        std::string source;
        std::string line;

        while (std::getline(stream, line)) {
            source += line + '\n';
        }

        // Load source
        ShaderProgramSource shaderSource = ParseShader(source);
        unsigned int shader = CreateShader(shaderSource.vertexSource, shaderSource.fragmentSource);

        shaderId = shader;
        shaderCreated = true;
    }

    void Shader::LoadShaderFromMemory(const std::string& source) {
        ShaderProgramSource shaderSource = ParseShader(source);
        unsigned int shader = CreateShader(shaderSource.vertexSource, shaderSource.fragmentSource);

        shaderId = shader;
        shaderCreated = true;
    }

    unsigned int Shader::GetId() {
        return shaderId;
    }

    // Uniforms
    int Shader::GetUniformLocation(const char* name) {
        return glGetUniformLocation(shaderId, name);
    }


    void Shader::SetUniformBool(const char* name, bool boolean) {
        glUniform1i(GetUniformLocation(name), boolean);
    }


    void Shader::SetUniform1f(const char* name, float x) {
        glUniform1f(GetUniformLocation(name), x);
    }

    void Shader::SetUniform2f(const char* name, float x, float y) {
        glUniform2f(GetUniformLocation(name), x, y);
    }

    void Shader::SetUniform3f(const char* name, float x, float y, float z) {
        glUniform3f(GetUniformLocation(name), x, y, z);
    }

    void Shader::SetUniform4f(const char* name, float x, float y, float z, float w) {
        glUniform4f(GetUniformLocation(name), x, y, z, w);
    }


    void Shader::SetUniform1i(const char* name, int x) {
        glUniform1i(GetUniformLocation(name), x);
    }

    void Shader::SetUniform2i(const char* name, int x, int y) {
        glUniform2i(GetUniformLocation(name), x, y);
    }

    void Shader::SetUniform3i(const char* name, int x, int y, int z) {
        glUniform3i(GetUniformLocation(name), x, y, z);
    }

    void Shader::SetUniform4i(const char* name, int x, int y, int z, int w) {
        glUniform4i(GetUniformLocation(name), x, y, z, w);
    }


    void Shader::SetUniformMat2fv(const char* name, glm::mat2 data) {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(data));
    }

    void Shader::SetUniformMat3fv(const char* name, glm::mat3 data) {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(data));
    }

    void Shader::SetUniformMat4fv(const char* name, glm::mat4 data) {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(data));
    }
}