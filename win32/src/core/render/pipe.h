#pragma once

#include "../include.h"
#include "render_type.h"
#include "render_target.h"

struct PipeState {
    PipeState()
    {
        memset(&mSSAO,          0,      sizeof(mSSAO));
        memset(&mGBuffer,       0,      sizeof(mGBuffer));
        memset(&mLightUBO,      0,      sizeof(mLightUBO));
        memset(&mShadowMap,     0,      sizeof(mShadowMap));
        memset(&mRenderTime,    0,      sizeof(mRenderTime));
        memset(&mPostScreen,    0,      sizeof(mPostScreen));
    }

    //  ��Դ����
    uint mLightUBO[3];

    struct RenderTime {
        //  ��¼��ǰ���ζ�����
        uint mVertexCount;
        //  ��¼��ǰ������Ⱦ��
        uint mDrawCount;
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
            , mDrawCount(0)
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
    MatrixStack mMaterialStack;

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

class Pipe {
public:
    virtual ~Pipe() {}
    virtual void OnAdd(PipeState * state) = 0;
    virtual void OnDel(PipeState * state) = 0;
    virtual void OnUpdate(PipeState * state) = 0;
};