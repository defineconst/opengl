#pragma once

#include "../include.h"
#include "render_type.h"
#include "render_target.h"

class PipeState {
public:
    PipeState();
    ~PipeState();

    //  ��Դ����
    uint mLightUBO[3];

    //  ��Ⱦʱ��Ϣ
    struct RenderTime {
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
        RenderTime()
            : mProgram(nullptr)
            , mCamera(nullptr)
            , mVertexCount(0)
            , mRenderCount(0)
            , mTexBase(0) 
        { }
    } mRenderTime;

    //  G-Buffer
    struct GBuffer {
        uint mPositionTexture;
        uint mDiffuseTexture;
        uint mNormalTexture;
    } mGBuffer;

    //  ��Ӱ��ͼ
    struct ShadowMap {
        uint mDirectTexture[LIMIT_LIGHT_DIRECT];
        uint mPointTexture[LIMIT_LIGHT_POINT];
        uint mSpotTexture[LIMIT_LIGHT_SPOT];
    } mShadowMap;

    //  ������Ļ
    struct PostScreen {
        uint mColorTexture;
        uint mDepthTexture;
    } mPostScreen;

    //  SSAO
    struct SSAO {
        uint mOcclusionTexture0;    //  һ�׶�(ģ��ǰ)
        uint mOcclusionTexture1;    //  ���׶�(ģ����)
    } mSSAO;

    //  ��Ⱦ������
    MatrixStack mMatrixStack;

    //  ��ȾĿ��
    RenderTarget mRenderTarget[2];

    //  �����Ⱦ����
    std::vector<CameraCommand> mCameraQueue;
    //  ��ȶ���
    std::vector<MaterialCommand> mDepthQueue;
    //  ��Ӱ�������
    std::vector<MaterialCommand> mShadowQueue;
    //  ��Դ���Ͷ���
    std::array<std::vector<LightCommand>, 3> mLightQueues;
    //  ������Ⱦ����
    std::array<std::vector<MaterialCommand>, 4> mForwardQueues;
    //  �ӳ���Ⱦ����
    std::array<std::vector<MaterialCommand>, 4> mDeferredQueues;
};

class Renderer;

class Pipe {
public:
    virtual ~Pipe() {}
    virtual void OnAdd(Renderer * renderer, PipeState * state) = 0;
    virtual void OnDel(Renderer * renderer, PipeState * state) = 0;
    virtual void OnUpdate(Renderer * renderer, PipeState * state) = 0;
};