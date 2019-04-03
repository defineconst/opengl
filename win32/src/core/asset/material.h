#pragma once

#include "asset.h"
#include "texture.h"

class Material {
public:
	Material(): mShininess(32.0f)
	{ }
	std::vector<Texture> mDiffuses;
	Texture mSpecular;				//	�߹���ͼ
	Texture mReflect;				//	������ͼ
	Texture mNormal;				//	������ͼ
    Texture mHeight;				//	�߶���ͼ
	float mShininess;				//	�����
};