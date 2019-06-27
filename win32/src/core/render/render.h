#pragma once

#include "../include.h"
#include "render_util.h"
#include "render_enum.h"
#include "render_target.h"

class Render {
public:
    static constexpr uint LIMIT_LIGHT_DIRECT = 1;
    static constexpr uint LIMIT_LIGHT_POINT = 2;
    static constexpr uint LIMIT_LIGHT_SPOT = 3;

    //  ��Ⱦ״̬
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

    struct BufferSet {
        //  G-Buffer
        struct GBuffer {
            uint mPositionTexture;
            uint mSpecularTexture;
            uint mDiffuseTexture;
            uint mNormalTexture;
            uint mDepthBuffer;
        } mGBuffer;

        //  ��Ӱ��ͼ
        struct ShadowMap {
            uint mDirectTexture[LIMIT_LIGHT_DIRECT];
            uint mPointTexture[LIMIT_LIGHT_POINT];
            uint mSpotTexture[LIMIT_LIGHT_SPOT];
        } mShadowMap;

        //  ��������
        struct OffScreen {
            uint mColorTexture;
            uint mDepthTexture;
        } mOffScreen;

        //  SSAO
        struct SSAO {
            uint mOcclusionTexture0;    //  һ�׶�(ģ��ǰ)
            uint mOcclusionTexture1;    //  ���׶�(ģ����)
        } mSSAO;
        
        //  ��Դ����
        uint mLightUBO[3];
        
        BufferSet()
        {
            memset(this, 0, sizeof(BufferSet));
        }
    };

public:
    Render();
    ~Render();

	MatrixStack & GetMatrixStack();
    //  ��Ⱦ
	void Once();
    //  ��Ⱦ�������
    void Post(const RenderCommand::TypeEnum type,
              const RenderCommand & command);

    const RenderState & GetRenderState() const { return _renderState; }

private:
    //  ��ʼ����Ⱦ����
    void InitRender();

    //  ������������
    void ClearCommands();

    //  Bind ϵ����.
    //      ��ϵ�к�����������ύͬʱӰ����Ⱦ���ڲ�״̬
    void Bind(const CameraCommand * command);
    bool Bind(const GLProgram     * program, uint pass);

    //  Post ϵ����.
    //      ��ϵ������������ύ, �����޸���Ⱦ���ڲ�״̬
    void Post(const Light * light);
    void Post(const GLMaterial * material);
    void Post(const glm::mat4 & transform);
    void Post(DrawTypeEnum drawType, const GLMesh * mesh);

    //  ���ݾ��������Դ
    void SortLightCommands();
    //  ������Ӱ
    void BakeLightDepthMap();
    void BakeLightDepthMap(Light * light, uint shadow);

    //  �������Ⱦ
    void RenderCamera();
    //  ��ȾGBuffer
    void RenderGBuffer();
    //  ��ȾSSAO
    void RenderSSAO();
    //  �ӳ���Ⱦ
    void RenderDeferred();
    //  ������Ⱦ
    void RenderForward();
    //  ������Ⱦ
    void RenderLightVolume(const LightCommand & command, uint shadow);

    //  ������Ⱦ���
    void PackUBOLightForward();
    void BindUBOLightForward();

private:
    RenderTarget _renderTarget[2];
    MatrixStack  _matrixStack;
    BufferSet    _bufferSet;
    GLMesh *     _screenQuad;
    GLProgram * _ssaoProgram;

    //  ״̬
    RenderState _renderState;
    //  �����Ⱦ����
    CameraCommandQueue _cameraQueue;
    //  SSAO����
    MaterialCommandQueue _ssaoQueue;
    //  ��Ӱ�������
    MaterialCommandQueue _shadowQueue;
    //  ��Դ���Ͷ���
    std::array<LightCommandQueue, 3> _lightQueues;
    //  ������Ⱦ����
    std::array<MaterialCommandQueue, 4> _forwardQueues;
    //  �ӳ���Ⱦ����
    std::array<MaterialCommandQueue, 4> _deferredQueues;
};

