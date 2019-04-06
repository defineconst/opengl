#pragma once

#include "component.h"
#include "../asset/material.h"
#include "../asset/texture.h"
#include "../asset/shader.h"
#include "../asset/mesh.h"

class Sprite : public Component {
public:
	Sprite();
	virtual ~Sprite() {}
	virtual void OnAdd() override;
	virtual void OnDel() override;
	virtual void OnUpdate(float dt) override;

	void AddMesh(Mesh * mesh, const Material & material)
	{
		_meshs.push_back(mesh);
		_mates.push_back(material);
	}

	void BindShader(const std::string & url);
	Shader * GetShader() { return _shader; }

protected:
	Shader * _shader;
	std::vector<Mesh *> _meshs;
	std::vector<Material> _mates;
};