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

    struct RenderState {
        //  ��¼��ǰ���ζ�����
        uint mVertexCount;
        //  ��¼��ǰ������Ⱦ��
        uint mRenderCount;
        //  ��¼��ǰTexture��ַ
        uint mTexBase;

        //  ��ǰ�󶨵�Program
        const GLProgram     * mProgram;
        //  ��ǰ�󶨵�Camera
        const CameraCommand * mCamera;
        RenderState()
            : mProgram(nullptr)
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
        uint mDepthBuffer;
        GBuffer()
            : mPositionTexture(0)
            , mSpecularTexture(0)
            , mDiffuseTexture(0)
            , mNormalTexture(0)
            , mDepthBuffer(0)
        { }
    };

    struct OffSceneBuffer {
        uint mColorTexture;
        uint mDepthTexture;
        OffSceneBuffer()
            : mColorTexture(0)
            , mDepthTexture(0)
        { }
    };

public:
    Render();
    ~Render();

	MatrixStack & GetMatrixStack();
    //  ��Ⱦ
	void RenderOnce();
    //  ��Ⱦ�������
    void PostCommand(const RenderCommand::TypeEnum type, const RenderCommand & command);

    const RenderState & GetRenderState() const { return _renderState; }

private:
    void StartRender();

    //  Bind ϵ����.
    //      ��ϵ�к�����������ύͬʱӰ����Ⱦ���ڲ�״̬
    void Bind(const CameraCommand * command);
    bool Bind(const GLProgram     * program, uint pass);

    //  Post ϵ����.
    //      ��ϵ������������ύ, �����޸���Ⱦ���ڲ�״̬
    void Post(const Light *light);
    void Post(const GLMaterial * material);
    void Post(const glm::mat4 & transform);
    void Post(DrawTypeEnum drawType, const GLMesh * mesh);

    void ClearCommands();

    //  �������Ⱦ
    void SortLightCommands();
    void BakeLightDepthMap();
    void BakeLightDepthMap(Light * light, uint shadow);

    void RenderCamera();
    void RenderForward();
    void RenderDeferred();
	void RenderForwardCommands(const MaterialCommandQueue & commands);
	void RenderDeferredCommands(const MaterialCommandQueue & commands);
    void RenderDeferredLightVolume(const LightCommand & command, uint shadow);

    //  ������Ⱦ���
    void PackUBOLightForward();
    void BindUBOLightForward();

private:
    RenderTarget    _renderTarget[2];
    MatrixStack     _matrixStack;

    //  ����buffer
    OffSceneBuffer _offSceneBuffer;
    //  �ӳ���Ⱦ
    GBuffer _bufferG;

    //  ״̬
    RenderState _renderState;
    //  �����Ⱦ����
    CameraCommandQueue _cameraQueue;
    //  ��Ӱ�������
    MaterialCommandQueue _shadowQueue;
    //  ��Դ���Ͷ���
    std::array<LightCommandQueue, 3> _lightQueues;
    //  ������Ⱦ����
    std::array<MaterialCommandQueue, 4> _forwardQueues;
    //  �ӳ���Ⱦ����
    std::array<MaterialCommandQueue, 4> _deferredQueues;

    //  �����ͼ
    uint _shadowMapDirect[LIMIT_LIGHT_DIRECT];
    uint _shadowMapPoint[LIMIT_LIGHT_POINT];
    uint _shadowMapSpot[LIMIT_LIGHT_SPOT];
    //  ��Դ����
    uint _lightForwardUBO[3];
};

