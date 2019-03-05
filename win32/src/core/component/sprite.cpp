#include "sprite.h"
#include "../mmc.h"
#include "../asset/file.h"
#include "../third/sformat.h"
#include "../render/render.h"

Sprite::Sprite()
	: _shader(nullptr)
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
	command.mCameraFlag = GetOwner()->GetCameraFlag();
	command.mCallFn = [this]() {
		//	�������
		glEnable(GL_BLEND);
		glBlendFunc(_blend.mSrc, _blend.mDst);
		//	������Ȳ���
		glEnable(GL_DEPTH_TEST);

		for (auto i = 0; i != _meshs.size(); ++i)
		{
			mmc::mRender.Bind(_shader);

			for (auto j = 0; j != _mates.at(i).mDiffuses.size(); ++j)
			{
				mmc::mRender.BindTexture(SFormat("material_.mDiffuse{0}", j), _mates.at(i).mDiffuses.at(j));
			}
			if (_mates.at(i).mParallax.GetBitmap() != nullptr)
			{
				mmc::mRender.BindTexture("material_.mParallax", _mates.at(i).mParallax);
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
			mmc::mRender.RenderIdx(_meshs.at(i)->GetGLID(), _meshs.at(i)->GetIdxCount());
		}

		//	�رջ��
		glDisable(GL_BLEND);
		//	�ر���Ȳ���
		glDisable(GL_DEPTH_TEST);
	};
	mmc::mRender.PostCommand(command);
}

void Sprite::BindShader(const std::string & url)
{
	_shader = File::LoadShader(url);
}
