#pragma once

#include "../include.h"
#include "render_type.h"
#include "render_target.h"

template <class T>
constexpr uint UBOTypeLen()
{
    return sizeof(T) > 16 ? 16
        : sizeof(T) > 8 ? 16
        : sizeof(T) > 4 ? 8
        : 4;
}

template <class T>
constexpr uint UBOOffsetOf(const uint base)
{
    return (UBOTypeLen<T>() + base - 1)
          / UBOTypeLen<T>()
          * UBOTypeLen<T>()
          + UBOTypeLen<T>();
}

class Render {
public:
    struct CameraInfo {
		enum Flag {
			kFLAG0 = 0x1,	kFLAG1 = 0x2,	kFLAG2 = 0x4,	kFLAG3 = 0x8,
			kFLAG4 = 0x10,	kFLAG5 = 0x20,	kFLAG6 = 0x30,	kFLAG7 = 0x40,
		};

		size_t mFlag;
		size_t mOrder;
		Camera * mCamera;
        CameraInfo(): mCamera(nullptr), mOrder(0), mFlag(0) { }
        CameraInfo(Camera * camera, size_t flag, size_t order) 
			: mCamera(camera), mFlag(flag), mOrder(order) { }
    };

	struct RenderInfo {
		size_t mVertexCount;
		size_t mRenderCount;
        size_t mTextureBase;
        size_t mTextureCount;
        const RenderPass * mPass;
		RenderInfo()
            : mPass(nullptr)
            , mVertexCount(0)
			, mRenderCount(0)
            , mTextureBase(0)
            , mTextureCount(0)
		{ }
	};

    //  ������Ⱦ��Դ������
    static constexpr uint LIMIT_FORWARD_LIGHT_DIRECT = 2;
    static constexpr uint LIMIT_FORWARD_LIGHT_POINT = 4;
    static constexpr uint LIMIT_FORWARD_LIGHT_SPOT = 4;

    //  ��Ӧ _uboLightForward[3]
    enum UBOLightForwardTypeEnum {
        kDIRECT,
        kPOINT,
        kSPOT,
    };

public:
    Render();
    ~Render();

	RenderMatrix & GetMatrix();

    //  ���
    void AddCamera(Camera * camera, size_t flag, size_t order = ~0);
	Camera *GetCamera(size_t order);
	void DelCamera(Camera * camera);
	void DelCamera(size_t order);

    //  ��Դ
	void AddLight(Light * light);
	void DelLight(Light * light);
    
    //  ��Ⱦ
	void RenderOnce();

	void PostCommand(const Shader * shader, const RenderCommand & command);

    //  ����
	const RenderInfo & GetRenderInfo() const { return _renderInfo; }

private:
    void Bind(Light * light);
    void Bind(CameraInfo * camera);
    bool Bind(const RenderPass * pass);
    void Bind(const Material * material);
    //	��ÿһ����Ⱦ�����ܱ仯�Ĳ���
    void BindEveryParam(CameraInfo * camera, Light * light, const RenderCommand & command);

    //	ִ�л�������
    void Draw(DrawTypeEnum drawType, const RenderMesh & mesh);

    void ClearCommands();

    //  ����ShadowMap
	void RenderShadow(Light * light);

    //  �������Ⱦ
    void RenderCamera(CameraInfo * camera);
    void RenderForward(CameraInfo * camera);
    void RenderDeferred(CameraInfo * camera);
	void RenderForwardCommands(CameraInfo * camera, Light * light, const RenderQueue & commands);
	void RenderDeferredCommands(CameraInfo * camera, Light * light, const RenderQueue & commands);

    //  ������Ⱦ��Դ���
    void InitUBOLightForward();
    void PackUBOLightForward();
    void BindUBOLightForward();

private:
    RenderMatrix    _matrix;
    RenderTarget    _shadowRT;
	RenderInfo      _renderInfo;

	std::vector<Light *>    _lights;
    std::vector<CameraInfo> _cameraInfos;

	//	��Ⱦ����, ��Ӱ����Ҫ���ֶ�������
    RenderQueue _shadowCommands;
    std::array<RenderQueue, 4> _forwardCommands;
    std::array<RenderQueue, 4> _deferredCommands;

    //  ��ԴUBO
    uint _uboLightForward[3];
};

