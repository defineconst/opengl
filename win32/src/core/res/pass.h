#pragma once

#include "res.h"
#include "../render/render_enum.h"

class Pass {
public:
    //  ���޳�
    bool    bCullFace;          //  �������޳�
    int     vCullFace;
    //  ���
    bool    bBlend;             //  �������
    int     vBlendSrc;
    int     vBlendDst;
    //  ��Ȳ���
    bool    bDepthTest;         //  ������Ȳ���
    bool    bDepthWrite;        //  �������д��
    //  ģ�����
    bool    bStencilTest;       //  ����ģ�����
    int     vStencilOpFail;		//	ģ�����ʧ��
    int     vStencilOpZFail;	//	��Ȳ���ʧ��
    int     vStencilOpZPass;	//	��Ȳ���ͨ��
	int		vStencilFunc;		//	ģ����Ժ���
    int     vStencilMask;       //  ģ�����ֵ
    int     vStencilRef;        //  ģ�����ֵ
    //  ��Ⱦ
	DrawTypeEnum		mDrawType;			//	draw����
    RenderTypeEnum		mRenderType;        //  ��Ⱦ����
    RenderQueueEnum     mRenderQueue;       //  ��Ⱦͨ��
	//	Shader ID
    uint  GLID;

    Pass() : GLID(0), bCullFace(false), bBlend(false), bDepthTest(false), bStencilTest(false)
    { }
};
