#include "sprite_batch.h"
#include "../mmc.h"
#include "../asset/file.h"
#include "../third/sformat.h"
#include "../render/render.h"

SpriteBatch::SpriteBatch(): _count(0)
{
}

SpriteBatch::~SpriteBatch()
{
}

void SpriteBatch::OnAdd()
{
}

void SpriteBatch::OnDel()
{
}

void SpriteBatch::OnUpdate(float dt)
{
	Render::Command command;
	command.mCameraFlag = GetOwner()->GetCameraFlag();
	command.mCallFn = [this]() {
		//	�������
		glEnable(GL_BLEND);
		glBlendFunc(_blend.mSrc, _blend.mDst);
		//	������Ȳ���
		glEnable(GL_DEPTH_TEST);

		mmc::mRender.Bind(_shader);
		for (auto i = 0; i != _meshs.size(); ++i)
		{
			for (auto j = 0; j != _mates.at(i).mDiffuses.size(); ++j)
			{
				mmc::mRender.BindTexture(SFormat("material_.mDiffuse{0}", j), _mates.at(i).mDiffuses.at(j));
			}
			if (_mates.at(i).mHeight.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mHeight", _mates.at(i).mHeight);
			}
			if (_mates.at(i).mSpecular.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mSpecular", _mates.at(i).mSpecular);
			}
			if (_mates.at(i).mReflect.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mReflect", _mates.at(i).mReflect);
			}
			if (_mates.at(i).mNormal.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mNormal", _mates.at(i).mNormal);
			}
			_shader->SetUniform("material_.mFlipUVX", _flipUVX);
			_shader->SetUniform("material_.mFlipUVY", _flipUVY);
			_shader->SetUniform("material_.mShininess", _mates.at(i).mShininess);
			mmc::mRender.RenderIdxInst(_meshs.at(i)->GetGLID(), _meshs.at(i)->GetIndices().size(), _count);
		}

		//	�رջ��
		glDisable(GL_BLEND);
		//	�ر���Ȳ���
		glDisable(GL_DEPTH_TEST);
	};
	mmc::mRender.PostCommand(command);
}
