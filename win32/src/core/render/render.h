#pragma once

#include "../include.h"
#include "render_util.h"
#include "render_enum.h"
#include "render_target.h"

class Render {
public:
    static constexpr uint LIMIT_LIGHT_DIRECT = 2;
    static constexpr uint LIMIT_LIGHT_POINT = 4;
    static constexpr uint LIMIT_LIGHT_SPOT = 4;

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
        uint mCountUseLightDirect;
        uint mCountUseLightPoint;
        uint mCountUseLightSpot;

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
        uint mSpecularTexture;
        uint mDiffuseTexture;
        uint mNormalTexture;
        uint mDepthTexture;
        GBuffer()
            : mPositionTexture(0)
            , mSpecularTexture(0)
            , mDiffuseTexture(0)
            , mNormalTexture(0)
            , mDepthTexture(0)
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
    void Post(const glm::mat4 & transform);

    //	ִ�л�������
    void Draw(DrawTypeEnum drawType, const Mesh & mesh);

    void ClearCommands();

    //  �������Ⱦ
    void SortLightCommands();
    void BakeLightDepthMap();
    void BakeLightDepthMap(Light * light);

    void RenderCamera();
    void RenderForward();
    void RenderDeferred();
	void RenderForwardCommands(const ObjectCommandQueue & commands);
	void RenderDeferredCommands(const ObjectCommandQueue & commands);
    void RenderLightVolume(const LightCommand & command, bool isRenderShadow);

    //  ������Ⱦ���
    void InitUBOLightForward();
    void PackUBOLightForward();
    void BindUBOLightForward();

    //  �ӳ���Ⱦ���
    void InitGBuffer();
    void FillGBuffer();

    template <class T, int N>
    bool IsEmptyQueueArray(const std::array<T, N> & ary);

private:
    RenderTarget    _renderTarget;
    MatrixStack     _matrixStack;
	RenderInfo      _renderInfo;

    //  ����б�
    std::vector<CameraInfo> _cameraInfos;

    //	��Ӱ�������
    ObjectCommandQueue _shadowCommands;
    //  ��������
    std::array<LightCommandQueue, 3> _lightQueues;
    //  ������Ⱦ����
    std::array<ObjectCommandQueue, 4> _forwardQueues;
    //  �ӳ���Ⱦ����
    std::array<ObjectCommandQueue, 4> _deferredQueues;

    //  ������Ⱦ
    uint _uboLightForward[3];
    //  �ӳ���Ⱦ
    GBuffer _gbuffer;
};

