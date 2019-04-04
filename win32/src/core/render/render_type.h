#pragma once

#include "../include.h"

class Mesh;
class Light;
class Camera;
class Shader;
class Bitmap;
class Texture;
class Material;
class BitmapCube;

class RenderMatrix {
public:
	enum ModeType { kPROJECT, kVIEW, kMODEL, };

public:
	RenderMatrix()
	{ }

	~RenderMatrix()
	{ }

	void Pop(ModeType mode)
	{
		GetStack(mode).pop();
	}

	void Push(ModeType mode)
	{
		GetStack(mode).push(GetStack(mode).top());
	}

	void Identity(ModeType mode)
	{
		GetStack(mode).push(glm::mat4(1));
	}

	void Mul(ModeType mode, const glm::mat4 & mat)
	{
		GetStack(mode).top() *= mat;
	}

	const glm::mat4 & Top(ModeType mode) const
	{
		return GetStack(mode).top();
	}

	const glm::mat4 & GetM() const
	{
		return Top(ModeType::kMODEL);
	}

	const glm::mat4 & GetV() const
	{
		return Top(ModeType::kVIEW);
	}

	const glm::mat4 & GetP() const
	{
		return Top(ModeType::kPROJECT);
	}

private:
	std::stack<glm::mat4> & GetStack(ModeType mode)
	{
		return _RenderMatrixs.at((size_t)mode);
	}

	const std::stack<glm::mat4> & GetStack(ModeType mode) const
	{
		return const_cast<RenderMatrix *>(this)->GetStack(mode);
	}

private:
	std::array<std::stack<glm::mat4>, 3> _RenderMatrixs;
};

struct RenderPass {
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

    RenderPass() : GLID(0), bCullFace(false), bBlend(false), bDepthTest(false), bStencilTest(false)
    { }
};

enum RenderQueueEnum {
    kBACKGROUND,    //  �ײ����
    kGEOMETRIC,     //  �������
    kOPACITY,       //  ͸������
    kTOP,           //  �������
};

enum RenderTypeEnum {
    kSHADOW,        //  ������Ӱ��ͼ
    kFORWARD,       //  ������Ⱦ
    kDEFERRED,      //  �ӳ���Ⱦ
};

//  ������Ⱦ������ṹ
struct RenderCommand {
    const RenderPass *  mPass;          //  �󶨵�Shader
    Mesh *              mMeshs;         //  �󶨵�����
    size_t              mMeshNum;       //  �󶨵���������
    Material *          mMaterials;     //  �󶨵Ĳ���
    size_t              mMaterialNum;   //  �󶨵Ĳ�������
    glm::mat4           mTransform;     //  �󶨵ı任����
    size_t              mCameraFlag;    //  �󶨵������ʶ
};

using RenderQueue = std::vector<RenderCommand>;