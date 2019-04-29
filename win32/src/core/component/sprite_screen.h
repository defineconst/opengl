#pragma once

#include "component.h"
#include "../asset/shader.h"
#include "../asset/material.h"

class SpriteScreen : public Component {
public:
	SpriteScreen();
	~SpriteScreen();
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;

	void BindShader(Shader * shader);
	void BindShader(const std::string & url);

	void ClearBitmap();
	void BindBitmap(Bitmap * bitmap);
	void BindBitmap(const std::string & url);

private:
	Shader * _shader;
	Material _material;
	RenderMesh _meshQuat;
};