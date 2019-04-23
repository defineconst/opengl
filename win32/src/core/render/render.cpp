#include "render.h"
#include "../asset/shader.h"
#include "../asset/material.h"
#include "../component/camera.h"
#include "../tools/glsl_tool.h"
#include "../tools/debug_tool.h"
#include "../component/light.h"
#include "../component/skybox.h"
#include "../component/transform.h"

Render::Render()
{ }

Render::~Render()
{
    //  �ͷ�����������Ⱦ�Ĺ�ԴUBO
    if (_uboLightForward[UBOLightForwardTypeEnum::kDIRECT] != 0 ||
        _uboLightForward[UBOLightForwardTypeEnum::kPOINT] != 0 ||
        _uboLightForward[UBOLightForwardTypeEnum::kSPOT] != 0)
    {
        assert(_uboLightForward[UBOLightForwardTypeEnum::kDIRECT] != 0);
        assert(_uboLightForward[UBOLightForwardTypeEnum::kPOINT] != 0);
        assert(_uboLightForward[UBOLightForwardTypeEnum::kSPOT] != 0);
        glDeleteBuffers(3, _uboLightForward);
    }
}

RenderMatrix & Render::GetMatrix()
{
    return _matrix;
}

void Render::AddCamera(Camera * camera, size_t flag, size_t order)
{
	assert(order == (size_t)-1 || GetCamera(order) == nullptr);
	auto fn = [order](const CameraInfo & info) { return order <= info.mOrder; };
	auto it = std::find_if(_cameraInfos.begin(), _cameraInfos.end(), fn);
	_cameraInfos.insert(it, CameraInfo(camera, flag, order));
}

Camera * Render::GetCamera(size_t order)
{
	auto fn = [order](const CameraInfo & info) { return info.mOrder == order; };
	auto it = std::find_if(_cameraInfos.begin(), _cameraInfos.end(), fn);
	return it != _cameraInfos.end() ? it->mCamera : nullptr;
}

void Render::DelCamera(Camera * camera)
{
	auto fn = [camera](const CameraInfo & info) { return info.mCamera == camera; };
	auto it = std::find_if(_cameraInfos.begin(), _cameraInfos.end(), fn);
	if (it != _cameraInfos.end()) _cameraInfos.erase(it);
	delete camera;
}

void Render::DelCamera(size_t order)
{
	auto fn = [order](const CameraInfo & info) { return info.mOrder == order; };
	auto it = std::remove_if(_cameraInfos.begin(), _cameraInfos.end(), fn);
	if (it != _cameraInfos.end())
	{ DelCamera(it->mCamera); }
}

void Render::AddLight(Light * light)
{
	_lights.push_back(light);
}

void Render::DelLight(Light * light)
{
	auto it = std::remove(_lights.begin(), _lights.end(), light);
	if (it != _lights.end()) { _lights.erase(it); }
}

void Render::RenderOnce()
{
	_renderInfo.mVertexCount = 0;
	_renderInfo.mRenderCount = 0;
	glClear(GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT |
			GL_STENCIL_BUFFER_BIT);
	//	���������ͼ
    for (auto & light : _lights)
    {
        RenderShadow(light);
    }

	//	�����ִ����Ⱦ����
	for (auto & camera : _cameraInfos)
	{
		Bind(&camera);
		RenderCamera(&camera);
		Bind((CameraInfo *)nullptr);
	}
	ClearCommands();
}

void Render::PostCommand(const Shader * shader, const RenderCommand & command)
{
    for (const auto & pass : shader->GetPasss())
    {
        auto cmd = command;
        cmd.mPass  = &pass;
        switch (cmd.mPass->mRenderType)
        {
        case RenderTypeEnum::kSHADOW:
            _shadowCommands.push_back(cmd); break;
        case RenderTypeEnum::kFORWARD:
            _forwardCommands.at(cmd.mPass->mRenderQueue).push_back(cmd); break;
        case RenderTypeEnum::kDEFERRED:
            _deferredCommands.at(cmd.mPass->mRenderQueue).push_back(cmd); break;
        }
    }
}

void Render::RenderShadow(Light * light)
{
    auto count = 0;
    _shadowRT.Beg();
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
	while (light->NextDrawShadow(count++, &_shadowRT))
	{
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);

		for (auto & command : _shadowCommands)
		{
            if (Bind(command.mPass))
            { 
                Bind(light);
            }

            BindEveryParam(nullptr, command);

            for (auto i = 0; i != command.mMeshNum; ++i)
            {
                Draw(command.mPass->mDrawType, command.mMeshs[i]);
            }
		}
	}
    _shadowRT.End();
}

void Render::RenderCamera(CameraInfo * camera)
{
    //  �ӳ���Ⱦ
    //      �����������Ⱦ
    //          ����GBuffer
    //      ���Դ��Ⱦ
    //          �󶨹�ԴUBO
    //          ��Ⱦ��Դ��Χ��
    
    //  �ӳ���Ⱦ
    _renderInfo.mPass = nullptr;
    RenderDeferred(camera);

	//  ������Ⱦ
    _renderInfo.mPass = nullptr;
	RenderForward(camera);

	//	���ڴ���
    _renderInfo.mPass = nullptr;
}

void Render::RenderForward(CameraInfo * camera)
{
    //  �����Դ����
    InitUBOLightForward();
    PackUBOLightForward();
    //  ��Ⱦ
    for (auto & commands : _forwardCommands)
    {
        RenderForwardCommands(camera, commands);
    }
}

void Render::RenderDeferred(CameraInfo * camera)
{
	
}

void Render::RenderForwardCommands(CameraInfo * camera, const RenderQueue & commands)
{
	for (const auto & command : commands)
	{
		if ((camera->mFlag & command.mCameraFlag) != 0)
		{
			if (Bind(command.mPass)) 
            {
                BindUBOLightForward();
            }
			
            BindEveryParam(camera, command);
			
            for (auto i = 0; i != command.mMeshNum; ++i)
			{
                Bind(&command.mMaterials[i]);

				Draw(command.mPass->mDrawType, command.mMeshs[i]);
			}
		}
	}
}

void Render::RenderDeferredCommands(CameraInfo * camera, Light * light, const RenderQueue & commands)
{
}

void Render::InitUBOLightForward()
{
/*
    layout (std140) uniform ExampleBlock
    {
                         // ��׼������       // ����ƫ����
        float value;     // 4               // 0 
        vec3 vector;     // 16              // 16  (������16�ı��������� 4->16)
        mat4 matrix;     // 16              // 32  (�� 0)
                         // 16              // 48  (�� 1)
                         // 16              // 64  (�� 2)
                         // 16              // 80  (�� 3)
        float values[3]; // 16              // 96  (values[0])
                         // 16              // 112 (values[1])
                         // 16              // 128 (values[2])
        bool boolean;    // 4               // 144
        int integer;     // 4               // 148
    };  
*/
    if (_uboLightForward[UBOLightForwardTypeEnum::kDIRECT] == 0 ||
        _uboLightForward[UBOLightForwardTypeEnum::kPOINT] == 0 ||
        _uboLightForward[UBOLightForwardTypeEnum::kSPOT] == 0)
    {
        assert(_uboLightForward[UBOLightForwardTypeEnum::kDIRECT] == 0);
        assert(_uboLightForward[UBOLightForwardTypeEnum::kPOINT] == 0);
        assert(_uboLightForward[UBOLightForwardTypeEnum::kSPOT] == 0);

        glGenBuffers(3, _uboLightForward);

        uint base = 0;
        //  �����UBO
        glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kDIRECT]);
        base = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mSMP)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mMatrix)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mNormal)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mAmbient)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mDiffuse)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mSpecular)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mPosition)>(base);
        base = glsl_tool::UBOOffsetBase<glm::vec4>(base);
        base = base * LIMIT_FORWARD_LIGHT_DIRECT;
        glBufferData(GL_UNIFORM_BUFFER, base, nullptr, GL_DYNAMIC_DRAW);

        //  ���ԴUBO
        base = 0;
        glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kPOINT]);
        base = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mSMP)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mK0)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mK1)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mK2)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mAmbient)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mDiffuse)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mSpecular)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mPosition)>(base);
        base = glsl_tool::UBOOffsetBase<glm::vec4>(base);
        base = base * LIMIT_FORWARD_LIGHT_POINT;
        glBufferData(GL_UNIFORM_BUFFER, base, nullptr, GL_DYNAMIC_DRAW);

        //  �۹��UBO
        base = 0;
        glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kSPOT]);
        base = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mSMP)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mK0)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mK1)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mK2)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mInCone)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mOutCone)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mMatrix)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mNormal)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mAmbient)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mDiffuse)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mSpecular)>(base);
        base = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mPosition)>(base);
        base = glsl_tool::UBOOffsetBase<glm::vec4>(base);
        base = base * LIMIT_FORWARD_LIGHT_SPOT;
        glBufferData(GL_UNIFORM_BUFFER, base, nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}

void Render::PackUBOLightForward()
{
    uint8_t directNum = 0;
    uint8_t pointNum = 0;
    uint8_t spotNum = 0;
    size_t directBase = 0;
    size_t pointBase = 0;
    size_t spotBase = 0;
    for (auto & light : _lights)
    {
        if (light->GetType() == Light::Type::kDIRECT && directNum++ >= LIMIT_FORWARD_LIGHT_DIRECT ||
            light->GetType() == Light::Type::kPOINT && pointNum++ >= LIMIT_FORWARD_LIGHT_POINT ||
            light->GetType() == Light::Type::kSPOT && spotNum++ >= LIMIT_FORWARD_LIGHT_SPOT)
        {
            continue;
        }

        switch (light->GetType())
        {
        case Light::Type::kDIRECT:
            {
                //  �����UBO
                auto direct = reinterpret_cast<LightDirect *>(light);
                glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kDIRECT]);

                directBase = glsl_tool::UBOOffsetBase<decltype(LightDirect::UBOData::mSMP)>(directBase);
                glBufferSubData(GL_UNIFORM_BUFFER, directBase, sizeof(LightDirect::UBOData::mSMP),        &direct->GetShadowMapPos());
                directBase = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mSMP)>(directBase);

                directBase = glsl_tool::UBOOffsetBase<decltype(LightDirect::UBOData::mMatrix)>(directBase);
                glBufferSubData(GL_UNIFORM_BUFFER, directBase, sizeof(LightDirect::UBOData::mMatrix),     &direct->GetMatrix());
                directBase = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mMatrix)>(directBase);

                directBase = glsl_tool::UBOOffsetBase<decltype(LightDirect::UBOData::mNormal)>(directBase);
                glBufferSubData(GL_UNIFORM_BUFFER, directBase, sizeof(LightDirect::UBOData::mNormal),     &direct->mNormal);
                directBase = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mNormal)>(directBase);

                directBase = glsl_tool::UBOOffsetBase<decltype(LightDirect::UBOData::mAmbient)>(directBase);
                glBufferSubData(GL_UNIFORM_BUFFER, directBase, sizeof(LightDirect::UBOData::mAmbient),    &direct->mAmbient);
                directBase = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mAmbient)>(directBase);

                directBase = glsl_tool::UBOOffsetBase<decltype(LightDirect::UBOData::mDiffuse)>(directBase);
                glBufferSubData(GL_UNIFORM_BUFFER, directBase, sizeof(LightDirect::UBOData::mDiffuse),    &direct->mDiffuse);
                directBase = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mDiffuse)>(directBase);

                directBase = glsl_tool::UBOOffsetBase<decltype(LightDirect::UBOData::mSpecular)>(directBase);
                glBufferSubData(GL_UNIFORM_BUFFER, directBase, sizeof(LightDirect::UBOData::mSpecular),   &direct->mSpecular);
                directBase = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mSpecular)>(directBase);

                directBase = glsl_tool::UBOOffsetBase<decltype(LightDirect::UBOData::mPosition)>(directBase);
                glBufferSubData(GL_UNIFORM_BUFFER, directBase, sizeof(LightDirect::UBOData::mPosition),   &direct->GetWorldPos());
                directBase = glsl_tool::UBOOffsetFill<decltype(LightDirect::UBOData::mPosition)>(directBase);

                directBase = glsl_tool::UBOOffsetBase<glm::vec4>(directBase);

                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
            break;
        case Light::Type::kPOINT:
            {
                //  ���ԴUBO
                auto point = reinterpret_cast<LightPoint *>(light);
                glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kPOINT]);

                pointBase = glsl_tool::UBOOffsetBase<decltype(LightPoint::UBOData::mSMP)>(pointBase);
                glBufferSubData(GL_UNIFORM_BUFFER, pointBase, sizeof(LightPoint::UBOData::mSMP),         &point->GetShadowMapPos());
                pointBase = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mSMP)>(pointBase);

                pointBase = glsl_tool::UBOOffsetBase<decltype(LightPoint::UBOData::mK0)>(pointBase);
                glBufferSubData(GL_UNIFORM_BUFFER, pointBase, sizeof(LightPoint::UBOData::mK0),          &point->mK0);
                pointBase = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mK0)>(pointBase);

                pointBase = glsl_tool::UBOOffsetBase<decltype(LightPoint::UBOData::mK1)>(pointBase);
                glBufferSubData(GL_UNIFORM_BUFFER, pointBase, sizeof(LightPoint::UBOData::mK1),          &point->mK1);
                pointBase = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mK1)>(pointBase);

                pointBase = glsl_tool::UBOOffsetBase<decltype(LightPoint::UBOData::mK2)>(pointBase);
                glBufferSubData(GL_UNIFORM_BUFFER, pointBase, sizeof(LightPoint::UBOData::mK2),          &point->mK2);
                pointBase = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mK2)>(pointBase);

                pointBase = glsl_tool::UBOOffsetBase<decltype(LightPoint::UBOData::mAmbient)>(pointBase);
                glBufferSubData(GL_UNIFORM_BUFFER, pointBase, sizeof(LightPoint::UBOData::mAmbient),     &point->mAmbient);
                pointBase = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mAmbient)>(pointBase);

                pointBase = glsl_tool::UBOOffsetBase<decltype(LightPoint::UBOData::mDiffuse)>(pointBase);
                glBufferSubData(GL_UNIFORM_BUFFER, pointBase, sizeof(LightPoint::UBOData::mDiffuse),     &point->mDiffuse);
                pointBase = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mDiffuse)>(pointBase);

                pointBase = glsl_tool::UBOOffsetBase<decltype(LightPoint::UBOData::mSpecular)>(pointBase);
                glBufferSubData(GL_UNIFORM_BUFFER, pointBase, sizeof(LightPoint::UBOData::mSpecular),    &point->mSpecular);
                pointBase = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mSpecular)>(pointBase);

                pointBase = glsl_tool::UBOOffsetBase<decltype(LightPoint::UBOData::mPosition)>(pointBase);
                glBufferSubData(GL_UNIFORM_BUFFER, pointBase, sizeof(LightPoint::UBOData::mPosition),    &point->GetWorldPos());
                pointBase = glsl_tool::UBOOffsetFill<decltype(LightPoint::UBOData::mPosition)>(pointBase);

                pointBase = glsl_tool::UBOOffsetBase<glm::vec4>(pointBase);

                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
            break;
        case Light::Type::kSPOT:
            {
                //  �۹��UBO
                auto spot = reinterpret_cast<LightSpot *>(light);
                glBindBuffer(GL_UNIFORM_BUFFER, _uboLightForward[kSPOT]);

                spotBase = glsl_tool::UBOOffsetBase<decltype(LightSpot::UBOData::mSMP)>(spotBase);
                glBufferSubData(GL_UNIFORM_BUFFER, spotBase, sizeof(LightSpot::UBOData::mSMP),          &spot->GetShadowMapPos());
                spotBase = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mSMP)>(spotBase);

                spotBase = glsl_tool::UBOOffsetBase<decltype(LightSpot::UBOData::mK0)>(spotBase);
                glBufferSubData(GL_UNIFORM_BUFFER, spotBase, sizeof(LightSpot::UBOData::mK0),           &spot->mK0);
                spotBase = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mK0)>(spotBase);

                spotBase = glsl_tool::UBOOffsetBase<decltype(LightSpot::UBOData::mK1)>(spotBase);
                glBufferSubData(GL_UNIFORM_BUFFER, spotBase, sizeof(LightSpot::UBOData::mK1),           &spot->mK1);
                spotBase = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mK1)>(spotBase);

                spotBase = glsl_tool::UBOOffsetBase<decltype(LightSpot::UBOData::mK2)>(spotBase);
                glBufferSubData(GL_UNIFORM_BUFFER, spotBase, sizeof(LightSpot::UBOData::mK2),           &spot->mK2);
                spotBase = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mK2)>(spotBase);

                spotBase = glsl_tool::UBOOffsetBase<decltype(LightSpot::UBOData::mInCone)>(spotBase);
                glBufferSubData(GL_UNIFORM_BUFFER, spotBase, sizeof(LightSpot::UBOData::mInCone),       &spot->mInCone);
                spotBase = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mInCone)>(spotBase);

                spotBase = glsl_tool::UBOOffsetBase<decltype(LightSpot::UBOData::mOutCone)>(spotBase);
                glBufferSubData(GL_UNIFORM_BUFFER, spotBase, sizeof(LightSpot::UBOData::mOutCone),      &spot->mOutCone);
                spotBase = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mOutCone)>(spotBase);

                spotBase = glsl_tool::UBOOffsetBase<decltype(LightSpot::UBOData::mMatrix)>(spotBase);
                glBufferSubData(GL_UNIFORM_BUFFER, spotBase, sizeof(LightSpot::UBOData::mMatrix),       &spot->GetMatrix());
                spotBase = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mMatrix)>(spotBase);

                spotBase = glsl_tool::UBOOffsetBase<decltype(LightSpot::UBOData::mNormal)>(spotBase);
                glBufferSubData(GL_UNIFORM_BUFFER, spotBase, sizeof(LightSpot::UBOData::mNormal),       &spot->mNormal);
                spotBase = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mNormal)>(spotBase);

                spotBase = glsl_tool::UBOOffsetBase<decltype(LightSpot::UBOData::mAmbient)>(spotBase);
                glBufferSubData(GL_UNIFORM_BUFFER, spotBase, sizeof(LightSpot::UBOData::mAmbient),      &spot->mAmbient);
                spotBase = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mAmbient)>(spotBase);

                spotBase = glsl_tool::UBOOffsetBase<decltype(LightSpot::UBOData::mDiffuse)>(spotBase);
                glBufferSubData(GL_UNIFORM_BUFFER, spotBase, sizeof(LightSpot::UBOData::mDiffuse),      &spot->mDiffuse);
                spotBase = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mDiffuse)>(spotBase);

                spotBase = glsl_tool::UBOOffsetBase<decltype(LightSpot::UBOData::mSpecular)>(spotBase);
                glBufferSubData(GL_UNIFORM_BUFFER, spotBase, sizeof(LightSpot::UBOData::mSpecular),     &spot->mSpecular);
                spotBase = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mSpecular)>(spotBase);

                spotBase = glsl_tool::UBOOffsetBase<decltype(LightSpot::UBOData::mPosition)>(spotBase);
                glBufferSubData(GL_UNIFORM_BUFFER, spotBase, sizeof(LightSpot::UBOData::mPosition),     &spot->GetWorldPos());
                spotBase = glsl_tool::UBOOffsetFill<decltype(LightSpot::UBOData::mPosition)>(spotBase);

                spotBase = glsl_tool::UBOOffsetBase<glm::vec4>(spotBase);

                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
            break;
        }
    }
}

void Render::BindUBOLightForward()
{
    static const std::tuple<const char *, uint, uint> FIND_TABLE[] = {
        { UBO_NAME_LIGHT_DIRECT, UniformBlockEnum::kLIGHT_DIRECT, UBOLightForwardTypeEnum::kDIRECT },
        { UBO_NAME_LIGHT_POINT, UniformBlockEnum::kLIGHT_POINT, UBOLightForwardTypeEnum::kPOINT },
        { UBO_NAME_LIGHT_SPOT, UniformBlockEnum::kLIGHT_SPOT, UBOLightForwardTypeEnum::kSPOT },
    };

    uint8_t directNum = 0;
    uint8_t pointNum = 0;
    uint8_t spotNum = 0;
    for (auto & light : _lights)
    {
        if (light->GetType() == Light::Type::kDIRECT && directNum++ != LIMIT_FORWARD_LIGHT_DIRECT ||
            light->GetType() == Light::Type::kPOINT && pointNum++ != LIMIT_FORWARD_LIGHT_POINT ||
            light->GetType() == Light::Type::kSPOT && spotNum++ != LIMIT_FORWARD_LIGHT_SPOT)
        {
            auto idx = glGetUniformBlockIndex(_renderInfo.mPass->GLID, std::get<0>(FIND_TABLE[light->GetType()]));

            glUniformBlockBinding(_renderInfo.mPass->GLID, idx, std::get<1>(FIND_TABLE[light->GetType()]));

            glBindBufferBase(GL_UNIFORM_BUFFER, std::get<1>(FIND_TABLE[light->GetType()]),
                             _uboLightForward[std::get<2>(FIND_TABLE[light->GetType()])]);
        }
    }

    Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_LIGHT_COUNT_DIRECT_, directNum);
    Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_LIGHT_COUNT_POINT_, pointNum);
    Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_LIGHT_COUNT_SPOT_, spotNum);

    //  ����Ӱ��ͼ
    auto count = _renderInfo.mTexBase;
    Shader::SetUniformTexArray2D(_renderInfo.mPass->GLID, UNIFORM_SHADOW_MAP_2D_, Light::GetShadowMap2D(), count++);
    Shader::SetUniformTexArray3D(_renderInfo.mPass->GLID, UNIFORM_SHADOW_MAP_3D_, Light::GetShadowMap3D(), count++);
    _renderInfo.mTexBase = count;
}

void Render::Bind(CameraInfo * camera)
{
	if (camera != nullptr)
	{
		mmc::mRender.GetMatrix().Identity(RenderMatrix::kVIEW);
		mmc::mRender.GetMatrix().Identity(RenderMatrix::kPROJ);
		mmc::mRender.GetMatrix().Mul(RenderMatrix::kVIEW, camera->mCamera->GetView());
		mmc::mRender.GetMatrix().Mul(RenderMatrix::kPROJ, camera->mCamera->GetProj());
		glViewport((int)camera->mCamera->GetViewport().x, (int)camera->mCamera->GetViewport().y,
				   (int)camera->mCamera->GetViewport().z, (int)camera->mCamera->GetViewport().w);
	}
	else
	{
		mmc::mRender.GetMatrix().Pop(RenderMatrix::kVIEW);
		mmc::mRender.GetMatrix().Pop(RenderMatrix::kPROJ);
	}
}

void Render::Bind(Light * light)
{
    assert(_renderInfo.mPass != nullptr);
    switch (light->GetType())
    {
    case Light::Type::kDIRECT:
        {
            auto idx = glGetUniformBlockIndex(_renderInfo.mPass->GLID, UBO_NAME_LIGHT_DIRECT);
            if (GL_INVALID_INDEX != idx)
            {
                glUniformBlockBinding(_renderInfo.mPass->GLID, idx, UniformBlockEnum::kLIGHT_DIRECT);
                glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_DIRECT, light->GetUniformBlock());
            }
        }
        break;
    case Light::Type::kPOINT:
        {
            auto idx = glGetUniformBlockIndex(_renderInfo.mPass->GLID, UBO_NAME_LIGHT_POINT);
            if (GL_INVALID_INDEX != idx)
            {
                glUniformBlockBinding(_renderInfo.mPass->GLID, idx, UniformBlockEnum::kLIGHT_POINT);
                glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_POINT, light->GetUniformBlock());
            }
        }
        break;
    case Light::Type::kSPOT:
        {
            auto idx = glGetUniformBlockIndex(_renderInfo.mPass->GLID, UBO_NAME_LIGHT_SPOT);
            if (GL_INVALID_INDEX != idx)
            {
                glUniformBlockBinding(_renderInfo.mPass->GLID, idx, UniformBlockEnum::kLIGHT_SPOT);
                glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_SPOT, light->GetUniformBlock());
            }
        }
        break;
    }
    Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_LIGHT_TYPE, light->GetType());
}

bool Render::Bind(const RenderPass * pass)
{
	if (_renderInfo.mPass != pass)
	{
		_renderInfo.mPass = pass;
        _renderInfo.mTexBase = 0;

		//	�������޳�
		if (pass->bCullFace)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(pass->vCullFace);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
		//	������ɫ���
		if (pass->bBlend)
		{
			glEnable(GL_BLEND);
			glBlendFunc(pass->vBlendSrc, pass->vBlendDst);
		}
		else
		{
			glDisable(GL_BLEND);
		}
		//	������Ȳ���
		if (pass->bDepthTest)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
		//	����ģ�����
		if (pass->bStencilTest)
		{
			glEnable(GL_STENCIL_TEST);
			glStencilMask(0xFF);
			glStencilFunc(pass->vStencilFunc, pass->vStencilRef, pass->vStencilMask);
			glStencilOp(pass->vStencilOpFail, pass->vStencilOpZFail, pass->vStencilOpZPass);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
		glUseProgram(pass->GLID);
		return true;
	}
	return false;
}

void Render::Bind(const Material * material)
{
    auto count = _renderInfo.mTexBase;

	for (auto i = 0; i != material->mDiffuses.size(); ++i)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, SFormat(UNIFORM_MATERIAL_DIFFUSE, i), material->mDiffuses.at(i), count++);
	}
	if (material->mSpecular != nullptr)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_SPECULAR, material->mSpecular, count++);
	}
	if (material->mReflect != nullptr)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_REFLECT, material->mReflect, count++);
	}
	if (material->mNormal != nullptr)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_NORMAL, material->mNormal, count++);
	}
	if (material->mHeight != nullptr)
	{
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_HEIGHT, material->mHeight, count++);
	}
    Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATERIAL_SHININESS, material->mShininess);
}

void Render::ClearCommands()
{
	_shadowCommands.clear();
	for (auto & queue : _forwardCommands) { queue.clear(); }
	for (auto & queue : _deferredCommands) { queue.clear(); }
}

void Render::BindEveryParam(CameraInfo * camera, const RenderCommand & command)
{
	auto & matrixM			= command.mTransform;
	auto & matrixV			= _matrix.GetV();
	auto & matrixP			= _matrix.GetP();
	const auto & matrixN	= glm::transpose(glm::inverse(glm::mat3(matrixM)));
	const auto & matrixMV	= matrixV * matrixM;
	const auto & matrixMVP	= matrixP * matrixMV;
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATRIX_N, matrixN);
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATRIX_M, matrixM);
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATRIX_V, matrixV);
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATRIX_P, matrixP);
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATRIX_MV, matrixMV);
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_MATRIX_MVP, matrixMVP);
	Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_GAME_TIME, glfwGetTime());
    if (camera != nullptr)
    {
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_CAMERA_POS, camera->mCamera->GetPos());
        Shader::SetUniform(_renderInfo.mPass->GLID, UNIFORM_CAMERA_EYE, camera->mCamera->GetEye());
    }
}

void Render::Draw(DrawTypeEnum drawType, const RenderMesh & mesh)
{
	assert(mesh.mVBO != 0);
	assert(mesh.mVAO != 0);
	glBindVertexArray(mesh.mVAO);
	switch (drawType)
	{
	case DrawTypeEnum::kINSTANCE:
		{
			//	TODO, �ݲ�ʵ��
		}
		break;
	case DrawTypeEnum::kVERTEX:
		{
			_renderInfo.mVertexCount += mesh.mVtxCount;
			glDrawArrays(GL_TRIANGLES, 0, mesh.mVtxCount);
		}
		break;
	case DrawTypeEnum::kINDEX:
		{
			_renderInfo.mVertexCount += mesh.mIdxCount;
			glDrawElements(GL_TRIANGLES, mesh.mIdxCount, GL_UNSIGNED_INT, nullptr);
		}
		break;
	}
	++_renderInfo.mRenderCount;
}
