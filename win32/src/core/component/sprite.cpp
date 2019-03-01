#include "sprite.h"
#include "../mmc.h"
#include "../asset/file.h"
#include "../third/sformat.h"
#include "../render/render.h"

Sprite::Sprite()
	: _shader(nullptr)
	, _showNormal(nullptr)
	, _flipUVX(0)
	, _flipUVY(0)
{
}

void Sprite::OnAdd()
{
}

void Sprite::OnDel()
{
}

void Sprite::OnUpdate(float dt)
{
	Render::Command command;
	command.mCameraIdx = GetOwner()->GetCameraIdx();
	command.mCallFn = [this]() {
		//	�������
		glEnable(GL_BLEND);
		glBlendFunc(_blend.mSrc, _blend.mDst);
		//	������Ȳ���
		glEnable(GL_DEPTH_TEST);

		mmc::mRender.Bind(_shader);
		for (auto i = 0; i != _meshs.size(); ++i)
		{
			for (auto j = 0; j != _materials.at(i).mDiffuses.size(); ++j)
			{
				mmc::mRender.BindTexture(SFormat("material_.mDiffuse{0}", j), _materials.at(i).mDiffuses.at(j));
			}
			if (_materials.at(i).mParallax.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mParallax", _materials.at(i).mParallax);
			}
			if (_materials.at(i).mSpecular.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mSpecular", _materials.at(i).mSpecular);
			}
			if (_materials.at(i).mReflect.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mReflect", _materials.at(i).mReflect);
			}
			if (_materials.at(i).mNormal.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mNormal", _materials.at(i).mNormal);
			}
			_shader->SetUniform("material_.mFlipUVX", _flipUVX);
			_shader->SetUniform("material_.mFlipUVY", _flipUVY);
			_shader->SetUniform("material_.mShininess", _materials.at(i).mShinines);
			mmc::mRender.RenderIdx(_meshs.at(i)->GetGLID(), _meshs.at(i)->GetIdxCount());
		}

		//	�رջ��
		glDisable(GL_BLEND);
		//	�ر���Ȳ���
		glDisable(GL_DEPTH_TEST);
	};
	mmc::mRender.PostCommand(command);
}
