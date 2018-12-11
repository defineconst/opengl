#pragma once

#include "../include.h"
#include "../math/vec4.h"
#include "../asset/texture.h"
#include "../render/render.h"

class Shader {
public:
    struct Info {
        bool mIsDTest;
        bool mIsDWrite;
        bool mIsZTest;
        bool mIsZWrite;
        Render::RenderQueueFlag mRenderFlag;
    };

public:
    Shader();
    ~Shader();

    bool InitFromFile(const std::string & vs, const std::string & fs);
    bool Init(const std::string & vs, const std::string & fs);
    bool Init(const char * vs, const char * fs);
    void Bind();

    void SetUniform(size_t idx, int val);
    void SetUniform(size_t idx, float val);
    void SetUniform(size_t idx, const Eigen::Vector3f & val);
    void SetUniform(size_t idx, const Eigen::Vector4f & val);
    void SetUniform(size_t idx, const Texture & val);

    void SetUniform(const std::string & key, int val);
    void SetUniform(const std::string & key, float val);
    void SetUniform(const std::string & key, const Eigen::Vector3f & val);
    void SetUniform(const std::string & key, const Eigen::Vector4f & val);
    void SetUniform(const std::string & key, const Texture & val);

    GLuint GetGLID() const { return _GLID; }

    Info & GetInfo() { return _info; }
private:
    GLuint _GLID;
    Info _info;
};