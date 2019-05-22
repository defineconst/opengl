#pragma once

#include "gl_res.h"

class GLProgram : public GLRes {
public:
    struct PassAttr {
        //  ���޳�
        int     vCullFace;
        //  ���
        int     vBlendSrc;
        int     vBlendDst;
        //  ��Ȳ���
        bool    bDepthTest;         //  ������Ȳ���
        bool    bDepthWrite;        //  �������д��
        //  ģ�����
        int     vStencilOpFail;		//	ģ�����ʧ��
        int     vStencilOpZFail;	//	��Ȳ���ʧ��
        int     vStencilOpZPass;	//	��Ȳ���ͨ��
        int		vStencilFunc;		//	ģ����Ժ���
        int     vStencilMask;       //  ģ�����ֵ
        int     vStencilRef;        //  ģ�����ֵ
        //  ��Ⱦ
        uint    vDrawType;			//	��������
        uint    vRenderType;        //  ��Ⱦ����
        uint    vRenderQueue;       //  ��Ⱦͨ��
    };
public:
    GLProgram()
    { }

    ~GLProgram()
    { 
        glDeleteProgram(_id);
    }

    void Init(const char * vString, const char * gString, const char * fString)
    {
        _id = glCreateProgram();

        if (vString != nullptr)
        {
            uint vs = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vs, 1, &vString, nullptr);
            glCompileShader(vs);
            AssertPass(vs, "VShader Error");
            glAttachShader(_id, vs);
            glDeleteShader(vs);
        }

        if (gString != nullptr)
        {
            uint gs = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(gs, 1, &gString, nullptr);
            glCompileShader(gs);
            AssertPass(gs, "GShader Error");
            glAttachShader(_id, gs);
            glDeleteShader(gs);
        }

        if (gString != nullptr)
        {
            uint fs = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fs, 1, &gString, nullptr);
            glCompileShader(fs);
            AssertPass(fs, "FShader Error");
            glAttachShader(_id, fs);
            glDeleteShader(fs);
        }

        iint ret;
        glLinkProgram(_id);
        glGetProgramiv(_id, GL_LINK_STATUS, &ret);
        ASSERT_LOG(ret != 0, "GLProgram Error");
    }

    void AddPassAttr(const PassAttr & passAttr)
    {
        _passAttrs.push_back(passAttr);
    }

    const PassAttr & GetPassAttr(uint i) const
    {
        return _passAttrs.at(i);
    }

    void UsePass(uint i) const
    {
        //  TODO
    }

    void Use() const
    {
        glUseProgram(_id);
    }

    //void Bind();

private:
    void AssertPass(uint shaderID, const std::string & errorTxt)
    {
        iint ret;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &ret);
        if (ret == 0)
        {
            char err[256] = { 0 };
            glGetShaderInfoLog(shaderID, sizeof(err), nullptr, err);
            ASSERT_LOG(false, "Shader Error. {0}, Code: {1}, Text: {2}", errorTxt, ret, err);
        }
    }

private:
    std::vector<PassAttr> _passAttrs;
    uint _id;
};