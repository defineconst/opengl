#pragma once

#include "asset.h"
#include "texture.h"
#include "bitmap_cube.h"

class Material {
public:
	Material(): mShininess(32.0f), mTexCube(nullptr)
	{ }
	std::vector<Bitmap *> mDiffuses;
	BitmapCube * mTexCube;			//	��������ͼ
	Bitmap * mSpecular;				//	�߹���ͼ
	Bitmap * mReflect;				//	������ͼ
	Bitmap * mNormal;				//	������ͼ
	Bitmap * mHeight;				//	�߶���ͼ
	float mShininess;				//	�����
};