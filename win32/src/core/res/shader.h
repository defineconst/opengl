#pragma once

#include "res.h"
#include "../render/render_enum.h"

class Pass {
public:
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
    int     vStencilOpFail;		//	ģ�����ʧ��
    int     vStencilOpZFail;	//	��Ȳ���ʧ��
    int     vStencilOpZPass;	//	��Ȳ���ͨ��
    int		vStencilFunc;		//	ģ����Ժ���
    int     vStencilMask;       //  ģ�����ֵ
    int     vStencilRef;        //  ģ�����ֵ
                                //  ��Ⱦ
    DrawTypeEnum		mDrawType;			//	draw����
    RenderTypeEnum		mRenderType;        //  ��Ⱦ����
    RenderQueueEnum     mRenderQueue;       //  ��Ⱦͨ��
                                            //	Shader ID
    uint  GLID;

    Pass() : GLID(0), bCullFace(false), bBlend(false), bDepthTest(false), bStencilTest(false)
    { }
};

class Bitmap;
class BitmapCube;

class Shader : public Res {
public:
    static void SetUniform(uint GLID, const std::string & key, iint val);
    static void SetUniform(uint GLID, const std::string & key, uint val);
	static void SetUniform(uint GLID, const std::string & key, float val);
	static void SetUniform(uint GLID, const std::string & key, double val);
	static void SetUniform(uint GLID, const std::string & key, const glm::vec3 & val);
	static void SetUniform(uint GLID, const std::string & key, const glm::vec4 & val);
	static void SetUniform(uint GLID, const std::string & key, const glm::mat3 & val);
	static void SetUniform(uint GLID, const std::string & key, const glm::mat4 & val);
    static void SetTexture2D(uint GLID, const std::string & key, const uint val, iint pos);
    static void SetTexture3D(uint GLID, const std::string & key, const uint val, iint pos);
    static void SetUniform(uint GLID, const std::string & key, const Bitmap * val, iint pos);
	static void SetUniform(uint GLID, const std::string & key, const BitmapCube * val, iint pos);

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

    const std::vector<Pass> & GetPasss() const
    {
        return _passs;
    }

    bool AddPass(
        const Pass & pass,
        const std::string & vs,
        const std::string & fs,
        const std::string & gs);

private:
    uint AddPass(const char * vs, 
				   const char * fs, 
				   const char * gs);

    void CheckPass(uint GLID, const std::string & string);

private:
    std::vector<Pass> _passs;
};