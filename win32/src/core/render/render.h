#pragma once

#include "../include.h"
#include "render_type.h"
#include "render_target.h"

class Render {
public:
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

    //  ��Ⱦ��Ϣ
	struct RenderInfo {
        //  ��¼��ǰ���ζ�����
		uint mVertexCount;
        //  ��¼��ǰ������Ⱦ��
		uint mRenderCount;
        //  ��¼��ǰTexture��ַ
        uint mTexBase;

        //  ������Ⱦ����
        uint mCountForwardLightDirect;
        uint mCountForwardLightPoint;
        uint mCountForwardLightSpot;

        //  ��ǰ�󶨵�pass
        const RenderPass * mPass;
        //  ��ǰ�󶨵�camera
        const CameraInfo * mCamera;
		RenderInfo()
            : mPass(nullptr)
            , mCamera(nullptr)
            , mVertexCount(0)
			, mRenderCount(0)
            , mTexBase(0) { }
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

	const RenderInfo & GetRenderInfo() const { return _renderInfo; }

private:
    //  Bind Function
    bool Bind(const RenderPass * pass);
    void Bind(const CameraInfo * camera);

    //  Post Function
    void Post(const Light * light);
    void Post(const Material * material);
    void Post(const RenderCommand & command);

    //	ִ�л�������
    void Draw(DrawTypeEnum drawType, const RenderMesh & mesh);

    void ClearCommands();

    //  ����ShadowMap
	void RenderShadow(Light * light);

    //  �������Ⱦ
    void RenderCamera();
    void RenderForward();
    void RenderDeferred();
	void RenderForwardCommands(const RenderQueue & commands);
	void RenderDeferredCommands(Light * light, const RenderQueue & commands);

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

