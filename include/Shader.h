#pragma once

#include <glad/glad.h>
#include "glm.hpp"

#include <filesystem>

class Shader
{
public:
    Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);

    GLuint getID() const
    {
        return ID;
    }

    void use() const
    {
        glUseProgram(ID);
    }

    void setBool(const std::string& name, const bool value) const;
    void setInt(const std::string& name, const int value) const;
    void setFloat(const std::string& name, const float value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const float x, const float y, const float z) const;
    void setVec3(const std::string& name, const glm::vec3& vec) const;
    void setVec2(const std::string& name, const float x, const float y) const;
    void setVec2(const std::string& name, const glm::vec2& vec) const;

private:
    GLuint ID;

    Shader() = delete;
    Shader(const Shader& rhs) = delete;
    Shader(Shader&& rhs) = delete;

    Shader& operator=(const Shader& rhs) = delete;
    Shader& operator=(const Shader&& rhs) = delete;
};