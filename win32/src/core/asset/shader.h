#pragma once

#include "asset.h"
#include "../render/render_type.h"

class Bitmap;
class Texture;
class BitmapCube;

class Shader : public Asset {
public:
    struct Pass {
        //  ���޳�
        bool    bCullFace;          //  �������޳�
        int     vCullFace;
        //  ���
        bool    bBlend;             //  �������
        int     vBlendSrc;
        int     vBlendDst;
        //  ��Ȳ���
        bool    bDepthTest;         //  ������Ȳ���
        bool    bDepthWrite;        //  �������д��
        //  ģ�����
        bool    bStencilTest;       //  ����ģ�����
        int     vStencilOpFail;
        int     vStencilOpZFail;
        int     vStencilOpZPass;
        int     vStencilMask;       //  ģ�����ֵ
        int     vStencilRef;        //  ģ�����ֵ
        //  ��Ⱦ
        int     mRenderType;        //  ��Ⱦ����
        int     mRenderQueue;       //  ��Ⱦͨ��
        GLuint  GLID;

        Pass() : GLID(0), bCullFace(false), bBlend(false), bDepthTest(false), bStencilTest(false)
        { }
    };
public:
    ~Shader();

    bool IsEmpty() const
    {
        return _passs.empty();
    }

    const Pass & GetPass(size_t idx) const
    {
        return _passs.at(idx);
    }

    bool AddPass(
        const Pass & pass,
        const std::string & vs,
        const std::string & fs,
        const std::string & gs);

    void SetUniform(size_t idx, int val);
    void SetUniform(size_t idx, float val);
    void SetUniform(size_t idx, double val);
    void SetUniform(size_t idx, const glm::vec3 & val);
    void SetUniform(size_t idx, const glm::vec4 & val);
    void SetUniform(size_t idx, const glm::mat3 & val);
    void SetUniform(size_t idx, const glm::mat4 & val);
    void SetUniform(size_t idx, const Bitmap * val, size_t pos);
    void SetUniform(size_t idx, const Texture & val, size_t pos);
    void SetUniform(size_t idx, const BitmapCube * val, size_t pos);

    void SetUniform(size_t pass, const std::string & key, int val);
    void SetUniform(size_t pass, const std::string & key, float val);
    void SetUniform(size_t pass, const std::string & key, double val);
    void SetUniform(size_t pass, const std::string & key, const glm::vec3 & val);
    void SetUniform(size_t pass, const std::string & key, const glm::vec4 & val);
    void SetUniform(size_t pass, const std::string & key, const glm::mat3 & val);
    void SetUniform(size_t pass, const std::string & key, const glm::mat4 & val);
    void SetUniform(size_t pass, const std::string & key, const Bitmap * val, size_t pos);
    void SetUniform(size_t pass, const std::string & key, const Texture & val, size_t pos);
    void SetUniform(size_t pass, const std::string & key, const BitmapCube * val, size_t pos);

private:
    GLuint AddPass(const char * vs, const char * fs, const char * gs);
    void CheckPass(GLuint GLID, const std::string & string);

private:
    std::vector<Pass> _passs;
};