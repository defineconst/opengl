#pragma once

#include "component.h"

class Mesh;
class Shader;
class RenderTarget;

class Light : public Component {
public:
	enum LightType {
		kDIRECT,
		kPOINT,
		kSPOT,
	};
public:
	Light(LightType type);
	virtual ~Light();

	virtual void OnAdd();
	virtual void OnDel();
	virtual void OnUpdate(float dt);
	LightType GetType() { return _type; }
	virtual RenderTarget * DrawShadow(bool onlyGet) = 0;

public:
	bool mIsDraw;
	glm::vec3 mAmbient;
	glm::vec3 mDiffuse;
	glm::vec3 mSpecular;

protected:
	RenderTarget * _shadowRT;

private:
	GLuint _vbo;
	GLuint _ebo;
	GLuint _vao;
	Shader * _shader;

private:
	LightType _type;
};

class LightDirect : public Light {
public:
	LightDirect(): Light(Light::kDIRECT)
	{ }

	~LightDirect()
	{ }

	void OpenShadow(std::uint32_t depthW, std::uint32_t depthH, 
					float orthoXMin, float orthoXMax, 
					float orthoYMin, float orthoYMax, 
					float orthoZMin, float orthoZMax,
					const glm::vec3 &up);
	void HideShadow();
	//	�����ͼ�ߴ�
	//	���������С
	virtual RenderTarget * DrawShadow(bool onlyGet) override;

public:
	glm::vec3 mNormal;

private:
	std::uint32_t _depthW;
	std::uint32_t _depthH;
	float _orthoXMin;
	float _orthoXMax;
	float _orthoYMin;
	float _orthoYMax;
	float _orthoZMin;
	float _orthoZMax;
	glm::vec3 _up;
};

class LightPoint : public Light {
public:
	LightPoint(): Light(Light::kPOINT)
	{ }

	~LightPoint()
	{ }

	virtual RenderTarget * DrawShadow(bool onlyGet) override
	{
		return _shadowRT;
	}

public:
	float mK0, mK1, mK2;
};

class LightSpot : public Light {
public:
	LightSpot(): Light(Light::kSPOT)
	{ }

	~LightSpot()
	{ }

	virtual RenderTarget * DrawShadow(bool onlyGet) override
	{
		return _shadowRT;
	}

public:
	glm::vec3 mNormal;
	float mK0, mK1, mK2;
	float mOutCone, mInCone;
};