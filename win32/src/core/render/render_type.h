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

enum DrawTypeEnum {
	kINSTANCE,		//	ʵ��draw
	kVERTEX,		//	����draw
	kINDEX,			//	����draw
};


class RenderMatrix {
public:
	enum ModeType { kPROJ, kVIEW, kMODEL, };

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
		return Top(ModeType::kPROJ);
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
    GLuint  GLID;

    RenderPass() : GLID(0), bCullFace(false), bBlend(false), bDepthTest(false), bStencilTest(false)
    { }
};

//  ������Ⱦ������ṹ
struct RenderCommand {
    const RenderPass *  mPass;          //  �󶨵�Shader
    const Mesh *        mMeshs;         //  �󶨵�����
	size_t              mMeshNum;       //  �󶨵���������
	const Material *    mMaterials;     //  �󶨵Ĳ���(������������������һ��)
	glm::mat4           mTransform;     //  �󶨵ı任����
    size_t              mCameraFlag;    //  �󶨵������ʶ
};

using RenderQueue = std::vector<RenderCommand>;