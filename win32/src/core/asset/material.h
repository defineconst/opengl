#pragma once

#include "asset.h"
#include "texture.h"

class Material {
public:
	Material(): mShinines(32.0f)
	{ }
	std::vector<Texture> mDiffuses;
	Texture mParallax;				//	�Ӳ���ͼ
	Texture mSpecular;				//	�߹���ͼ
	Texture mReflect;				//	������ͼ
	Texture mNormal;				//	������ͼ
	float mShinines;				//	�����
};