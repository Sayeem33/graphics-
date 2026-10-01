#pragma once
#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/gl.h>
#include "math3d.hpp"
#include <string>
#include <iostream>
#include <unordered_map>

class Shader {
public:
    GLuint id{0};
    mutable std::unordered_map<std::string, GLint> uniformLocations;

    Shader() = default;
    Shader(const char* vertexSrc, const char* fragmentSrc) {
        init(vertexSrc, fragmentSrc);
    }

    ~Shader() {
        if (id != 0) {
            glDeleteProgram(id);
            id = 0;
        }
    }

    // Disable copy, allow move
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& o) noexcept : id(o.id), uniformLocations(std::move(o.uniformLocations)) {
        o.id = 0;
    }
    Shader& operator=(Shader&& o) noexcept {
        if (this != &o) {
            if (id != 0) glDeleteProgram(id);
            id = o.id;
            uniformLocations = std::move(o.uniformLocations);
            o.id = 0;
        }
        return *this;
    }

    bool init(const char* vertexSrc, const char* fragmentSrc) {
        GLuint vShader = compileShader(GL_VERTEX_SHADER, vertexSrc);
        if (!vShader) return false;

        GLuint fShader = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);
        if (!fShader) {
            glDeleteShader(vShader);
            return false;
        }

        id = glCreateProgram();
        glAttachShader(id, vShader);
        glAttachShader(id, fShader);
        glLinkProgram(id);

        GLint success;
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success) {
            char log[1024];
            glGetProgramInfoLog(id, 1024, nullptr, log);
            std::cerr << "[SHADER LINK ERROR]:\n" << log << std::endl;
            glDeleteProgram(id);
            id = 0;
        }

        glDeleteShader(vShader);
        glDeleteShader(fShader);
        return id != 0;
    }

    void use() const {
        if (id != 0) glUseProgram(id);
    }

    GLint getUniformLocation(const std::string& name) const {
        auto it = uniformLocations.find(name);
        if (it != uniformLocations.end()) {
            return it->second;
        }
        GLint loc = glGetUniformLocation(id, name.c_str());
        uniformLocations[name] = loc;
        return loc;
    }

    void setMat4(const std::string& name, const m3d::Mat4& mat) const {
        GLint loc = getUniformLocation(name);
        if (loc != -1) {
            glUniformMatrix4fv(loc, 1, GL_FALSE, mat.data());
        }
    }

    void setVec3(const std::string& name, const m3d::Vec3& v) const {
        GLint loc = getUniformLocation(name);
        if (loc != -1) {
            glUniform3f(loc, v.x, v.y, v.z);
        }
    }

    void setVec3(const std::string& name, float x, float y, float z) const {
        GLint loc = getUniformLocation(name);
        if (loc != -1) {
            glUniform3f(loc, x, y, z);
        }
    }

    void setVec4(const std::string& name, const m3d::Vec4& v) const {
        GLint loc = getUniformLocation(name);
        if (loc != -1) {
            glUniform4f(loc, v.x, v.y, v.z, v.w);
        }
    }

    void setFloat(const std::string& name, float val) const {
        GLint loc = getUniformLocation(name);
        if (loc != -1) {
            glUniform1f(loc, val);
        }
    }

    void setInt(const std::string& name, int val) const {
        GLint loc = getUniformLocation(name);
        if (loc != -1) {
            glUniform1i(loc, val);
        }
    }

    void setBool(const std::string& name, bool val) const {
        setInt(name, val ? 1 : 0);
    }

private:
    GLuint compileShader(GLenum type, const char* src) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, log);
            std::cerr << "[SHADER COMPILE ERROR (" << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << ")]:\n"
                      << log << std::endl;
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }
};

#endif // SHADER_HPP
