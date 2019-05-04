#pragma once

#include "component.h"
#include "../res/mesh.h"
#include "../res/bitmap.h"
#include "../res/bitmap_cube.h"

class Shader;
class RenderTarget;

class Light : public Component {
public:
	enum Type {
		kDIRECT,
		kPOINT,
		kSPOT,
	};

private:
    static std::weak_ptr<Mesh> s_directVolmue;
    static std::weak_ptr<Mesh> s_pointVolmue;
    static std::weak_ptr<Mesh> s_spotVolmue;
    std::shared_ptr<Mesh> NewVolume();

public:
    Light(Type type);

    virtual ~Light()
    {
        glDeleteBuffers(1, &_uniformBlock);
        glDeleteTextures(1, &_shadowMap);
    }

    virtual void OnAdd() {}
    virtual void OnDel() {}
    virtual void OnUpdate(float dt);
    virtual bool NextDrawShadow(uint count, RenderTarget * rt) = 0;

    const Type & GetType() const { return _type; }
    const Mesh & GetMesh() const { return *_volume; }
    const uint & GetSMP() const { return _shadowMap; }
    const uint & GetUBO() const { return _uniformBlock; }
    static float CalLightDistance(float k0, float k1, float k2, float s);

public:
    glm::mat4 mMatrix;
    glm::vec3 mAmbient;
	glm::vec3 mDiffuse;
	glm::vec3 mSpecular;
    glm::vec3 mPosition;

protected:
    //  ��ԴͶӰ����
    glm::mat4 _proj;
    //  shadow map
    uint _shadowMap;
    //  UBO
    uint _uniformBlock;
    //  �����
    std::shared_ptr<Mesh> _volume;

    Shader * _shader;
private:
	Type _type;
};

class LightDirect : public Light {
public:
    //  ����ṹ���������˵������, �ýṹ��ӦShader��Ķ���
    struct UBOData {
        glm::mat4 mMatrix;
        glm::vec3 mNormal;
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        glm::vec3 mPosition;
    };

public:
    static uint GetUBOLength();

public:
	LightDirect(): Light(Light::kDIRECT)
	{ }

    ~LightDirect()
    { }

	void OpenShadow(const glm::vec2 & orthoX,	//	����
					const glm::vec2 & orthoY,	//	����
					const glm::vec2 & orthoZ);	//	ǰ��

	virtual bool NextDrawShadow(uint count, RenderTarget * rt) override;

public:
    glm::vec3 mNormal;
};

class LightPoint : public Light {
public:
    //  ����ṹ���������˵������, �ýṹ��ӦShader��Ķ���
    struct UBOData {
        float mFar, mNear;
        float mK0, mK1, mK2;
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        glm::vec3 mPosition;
    };

    static uint GetUBOLength();

public:
	LightPoint(): Light(Light::kPOINT)
	{ }

    ~LightPoint()
    { }

	void OpenShadow(const float n, const float f);

    virtual bool NextDrawShadow(uint count, RenderTarget * rt) override;

public:
    float mFar, mNear;
    float mK0, mK1, mK2;
};

class LightSpot : public Light {
public:
    //  ����ṹ���������˵������, �ýṹ��ӦShader��Ķ���
    struct UBOData {
        float mK0, mK1, mK2;
        float mInCone, mOutCone;
        glm::mat4 mMatrix;
        glm::vec3 mNormal;
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        glm::vec3 mPosition;
    };

    static uint GetUBOLength();

public:
	LightSpot(): Light(Light::kSPOT)
	{ }

    ~LightSpot()
    { }

	void OpenShadow(const float n, const float f);

    virtual bool NextDrawShadow(uint count, RenderTarget * rt) override;

public:
	glm::vec3 mNormal;
    float mK0, mK1, mK2;
	float mOutCone, mInCone;
};