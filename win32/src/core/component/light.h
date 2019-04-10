#pragma once

#include "component.h"
#include "../asset/bitmap.h"
#include "../asset/bitmap_cube.h"

class RenderTarget;

class Light : public Component {
public:
	enum Type {
		kDIRECT,
		kPOINT,
		kSPOT,
	};

public:
	Light(Type type)
        : _type(type), _blockID(0)
    { }

    virtual ~Light()
    {
        glDeleteBuffers(1, &_blockID);
    }

	virtual void OnAdd();
	virtual void OnDel();
	virtual void OnUpdate(float dt) {	}
    virtual bool NextDrawShadow(size_t count, RenderTarget * rt) = 0;

    const glm::vec3 & GetPos() const { return _position; }
    GLuint GetBlockID() const { return _blockID; }
    Type GetType() const { return _type; }

public:
	glm::vec3 mAmbient;
	glm::vec3 mDiffuse;
	glm::vec3 mSpecular;

protected:
    //  UBO
    GLuint _blockID;
    //  ��������, ��������ÿ��NextDrawShadow�����
    glm::vec3 _position;
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
    };

public:
	LightDirect()
        : Light(Light::kDIRECT)
        , mShadowTex(nullptr)
	{ }

	~LightDirect();

	void OpenShadow(const std::uint32_t depthW, 
					const std::uint32_t depthH,
					const glm::vec2 & orthoX,	//	����
					const glm::vec2 & orthoY,	//	����
					const glm::vec2 & orthoZ);	//	ǰ��

	virtual bool NextDrawShadow(size_t count, RenderTarget * rt) override;

public:
    glm::vec3 mNormal;
    
    RenderTexture2D * mShadowTex;

private:
	std::uint32_t _depthW;
	std::uint32_t _depthH;
	glm::mat4 _proj;
};

class LightPoint : public Light {
public:
    //  ����ṹ���������˵������, �ýṹ��ӦShader��Ķ���
    struct UBOData {
        float mK0, mK1, mK2;
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        glm::vec3 mPosition;
    };

public:
	LightPoint()
		: Light(Light::kPOINT)
        , mShadowTex(nullptr)
	{ }

    ~LightPoint();

	void OpenShadow(const std::uint32_t depthW,
					const std::uint32_t depthH, 
					const float n, const float f);

    virtual bool NextDrawShadow(size_t count, RenderTarget * rt) override;

public:
	float mK0, mK1, mK2;

    RenderTexture3D * mShadowTex;

private:
	std::uint32_t _depthW;
	std::uint32_t _depthH;
	glm::mat4 _proj;
};

class LightSpot : public Light {
public:
    //  ����ṹ���������˵������, �ýṹ��ӦShader��Ķ���
    struct UBOData {
        float mK0, mK1, mK2;
        float mInCone, mOutCone;
        glm::vec3 mNormal;
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        glm::vec3 mPosition;
    };

public:
	LightSpot()
		: Light(Light::kSPOT)
		, mShadowTex(nullptr)
	{ }

    ~LightSpot();

	void OpenShadow(const std::uint32_t depthW, 
					const std::uint32_t depthH,
					const float n, const float f);

    virtual bool NextDrawShadow(size_t count, RenderTarget * rt) override;

public:
	glm::vec3 mNormal;
    float mK0, mK1, mK2;
	float mOutCone, mInCone;
    RenderTexture2D * mShadowTex;

private:
	std::uint32_t _depthW;
	std::uint32_t _depthH;
    glm::mat4 _proj;
};