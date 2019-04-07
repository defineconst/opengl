#pragma once

#include "asset.h"
#include "texture.h"
#include "bitmap_cube.h"

class Material {
public:
	Material(): mShininess(32.0f), mTexCube(nullptr)
	{ }
	std::vector<Texture> mDiffuses;
	BitmapCube * mTexCube;			//	��������ͼ
	Texture mSpecular;				//	�߹���ͼ
	Texture mReflect;				//	������ͼ
	Texture mNormal;				//	������ͼ
    Texture mHeight;				//	�߶���ͼ
	float mShininess;				//	�����
};