#include "sprite_outline.h"
#include "../mmc.h"
#include "../render/render.h"

void SpriteOutline::OnAdd()
{
}

void SpriteOutline::OnDel()
{
}

void SpriteOutline::OnUpdate(float dt)
{
	Render::Command command1;
	command1.mCameraFlag = GetOwner()->GetCameraFlag();
	command1.mCallFn = [this]() {
		//	����ģ�����
		glEnable(GL_STENCIL_TEST);
		glStencilMask(0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
	};
	mmc::mRender.PostCommand(command1);

	Sprite::OnUpdate(dt);

	Render::Command command2;
	command2.mCameraFlag = GetOwner()->GetCameraFlag();
	command2.mCallFn = [this]() {
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		mmc::mRender.GetMatrix().Push(RenderMatrix::kMODEL);
		mmc::mRender.GetMatrix().Mul(RenderMatrix::kMODEL, glm::scale(glm::mat4(1), glm::vec3(_width)));
		mmc::mRender.Bind(_outline);
		for (auto i = 0; i != _meshs.size(); ++i)
		{
			mmc::mRender.RenderIdx(_meshs.at(i)->GetGLID(), _meshs.at(i)->GetIdxCount());
		}
		mmc::mRender.GetMatrix().Pop(RenderMatrix::kMODEL);
		glDisable(GL_STENCIL_TEST);
	};
	mmc::mRender.PostCommand(command2);
}
