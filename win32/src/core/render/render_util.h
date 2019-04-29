#pragma once

#include "../include.h"

class Mesh;
class Pass;
class Light;
class Camera;
class Shader;
class Bitmap;
class Material;
class BitmapCube;

class MatrixStack {
public:
	enum ModeType { kPROJ, kVIEW, kMODEL, };

public:
	MatrixStack()
	{ }

	~MatrixStack()
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
	std::stack<glm::mat4> & GetStack(ModeType mode) const
	{
		return _matrixs.at((size_t)mode);
	}

private:
	mutable std::array<std::stack<glm::mat4>, 3> _matrixs;
};

//  ������Ⱦ������ṹ
struct RenderCommand {
    const Pass * mPass;             //  �󶨵�Shader
    const Mesh * mMeshs;            //  �󶨵�����
    uint mMeshNum;                  //  �󶨵���������
    const Material * mMaterials;    //  �󶨵Ĳ���(������������������һ��)
    glm::mat4 mTransform;           //  �󶨵ı任����
    uint mCameraFlag;               //  �󶨵������ʶ
};

using RenderQueue = std::vector<RenderCommand>;