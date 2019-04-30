#pragma once

#include "../include.h"
#include "render_util.h"
#include "render_enum.h"
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
        const Pass * mPass;
        //  ��ǰ�󶨵�camera
        const CameraInfo * mCamera;
		RenderInfo()
            : mPass(nullptr)
            , mCamera(nullptr)
            , mVertexCount(0)
			, mRenderCount(0)
            , mTexBase(0) { }
	};

    struct GBuffer {
        uint mPositionTexture;
        uint mSpeculerTexture;
        uint mDiffuseTexture;
        uint mNormalTexture;
        uint mDepthBuffer;
        GBuffer()
            : mPositionTexture(0)
            , mSpeculerTexture(0)
            , mDiffuseTexture(0)
            , mNormalTexture(0)
            , mDepthBuffer(0)
        { }
    };

public:
    Render();
    ~Render();

	MatrixStack & GetMatrixStack();

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
    bool Bind(const Pass * pass);
    void Bind(const CameraInfo * camera);

    //  Post Function
    void Post(const Light * light);
    void Post(const Material & material);
    void Post(const RenderCommand & command);

    //	ִ�л�������
    void Draw(DrawTypeEnum drawType, const Mesh & mesh);

    void ClearCommands();

    //  ����ShadowMap
	void RenderShadow(Light * light);

    //  �������Ⱦ
    void RenderCamera();
    void RenderForward();
    void RenderDeferred();
	void RenderForwardCommands(const RenderQueue & commands);
	void RenderDeferredCommands(Light * light, const RenderQueue & commands);

    //  ������Ⱦ���
    void InitUBOLightForward();
    void PackUBOLightForward();
    void BindUBOLightForward();

    //  �ӳ���Ⱦ���
    void InitGBuffer();
    void BegGBuffer();
    void EndGBuffer();

private:
    RenderTarget    _renderTarget;
    MatrixStack     _matrixStack;
	RenderInfo      _renderInfo;

    //  ��Դ�б�
	std::vector<Light *>    _lights;
    //  ����б�
    std::vector<CameraInfo> _cameraInfos;
	//	��Ӱ�������
    RenderQueue _shadowCommands;
    //  ������Ⱦ����
    std::array<RenderQueue, 4> _forwardCommands;
    //  �ӳ���Ⱦ����
    std::array<RenderQueue, 4> _deferredCommands;

    //  ������Ⱦ
    uint _uboLightForward[3];

    //  �ӳ���Ⱦ
    GBuffer _gbuffer;
};

