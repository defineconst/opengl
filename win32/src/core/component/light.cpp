#include "light.h"
#include "transform.h"
#include "../render/render.h"
#include "../cfg/cfg_cache.h"
#include "../tools/glsl_tool.h"

uint Light::s_VIEW_W = 0;
uint Light::s_VIEW_H = 0;
Light::LightPool Light::s_lightPool;

void Light::LightPool::Clear()
{
    glDeleteTextures(1, &_tex2D);
    glDeleteTextures(1, &_tex3D);
    glDeleteBuffers(_lenUBO, _uboArray.data());

    _lenUBO = _len2D = _len3D = 0;
    _tex2D = _tex3D = 0;
    _posStock2D.clear();
    _posStock3D.clear();
    _uboArray.clear();
    _uboStock.clear();
}

uint Light::LightPool::NewUBO()
{
    AllocUBO();

    auto top = _uboStock.back();
    _uboStock.pop_back();
    return top;
}

uint Light::LightPool::NewPos2D()
{
    AllocPos2D();

    auto top = _posStock2D.back();
    _posStock2D.pop_back();
    return top;
}

uint Light::LightPool::NewPos3D()
{
    AllocPos3D();

    auto top = _posStock3D.back();
    _posStock3D.pop_back();
    return top;
}

void Light::LightPool::FreeUBO(uint pos)
{
    _uboStock.push_back(pos);
    
    if (_uboStock.size() == _lenUBO)
    {
        glDeleteBuffers(_lenUBO, _uboArray.data());
        _uboStock.clear();
        _uboArray.clear();
        _lenUBO = 0;
    }
}

void Light::LightPool::FreePos2D(uint pos)
{
    _posStock2D.push_back(pos);

    if (_posStock2D.size() == _len2D)
    {
        glDeleteTextures(1, &_tex2D);
        _tex2D = _len2D = 0;
        _posStock2D.clear();
    }
}

void Light::LightPool::FreePos3D(uint pos)
{
    _posStock3D.push_back(pos);

    if (_posStock3D.size() == _len3D)
    {
        glDeleteTextures(1, &_tex3D);
        _tex2D = _len3D = 0;
        _posStock3D.clear();
    }
}

void Light::LightPool::AllocUBO()
{
    if (_uboStock.empty())
    {
        glDeleteBuffers(_lenUBO, _uboArray.data());
        std::generate_n(std::back_inserter(_uboStock), 
                        LightPool::ALLOC_LENGTH_STEP,
                        [this](){return _lenUBO++;});

        _uboArray.resize(_lenUBO);
        glGenBuffers(_lenUBO, &_uboArray[0]);
    }
}

void Light::LightPool::AllocPos2D()
{
    if (_tex2D == 0)
    {
        s_VIEW_W = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "w")->ToInt();
        s_VIEW_H = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "h")->ToInt();
        glGenTextures(1, &_tex2D);
    }
    if (_posStock2D.empty())
    {
        std::generate_n(std::back_inserter(_posStock2D), 
                        LightPool::ALLOC_LENGTH_STEP, 
                        [this]() {return _len2D++;});

        glBindTexture(GL_TEXTURE_2D_ARRAY, _tex2D);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, s_VIEW_W, s_VIEW_H,
                     _len2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
}

void Light::LightPool::AllocPos3D()
{
    if (_tex3D == 0)
    {
        s_VIEW_W = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "w")->ToInt();
        s_VIEW_H = Global::Ref().RefCfgCache().At("init")->At("shadow_map", "w")->ToInt();
        glGenTextures(1, &_tex3D);
    }
    if (_posStock3D.empty())
    {
        std::generate_n(std::back_inserter(_posStock3D),
                        LightPool::ALLOC_LENGTH_STEP, 
                        [this]() {return _len3D++;});

        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, _tex3D);
        glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT, s_VIEW_W, s_VIEW_H,
                     _len3D * 6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
    }
}

//  --------------------------------------------------------------------------------
//  ��Դʵ��
void Light::OnAdd()
{
	Global::Ref().RefRender().AddLight(this);
}

void Light::OnDel()
{
	Global::Ref().RefRender().DelLight(this);
}

uint LightDirect::GetUBOLength()
{
    auto base = glsl_tool::UBOOffsetFill<decltype(UBOData::mSMP)>(0);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mMatrix)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mNormal)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mAmbient)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mDiffuse)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mSpecular)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mPosition)>(base);
    return glsl_tool::UBOOffsetBase<glm::vec4>(base);
}

void LightDirect::OpenShadow(
	const glm::vec2 & orthoX,
	const glm::vec2 & orthoY,
	const glm::vec2 & orthoZ)
{
    glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
    glBufferData(GL_UNIFORM_BUFFER, GetUBOLength(), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    _proj = glm::ortho(orthoX.x, orthoX.y, orthoY.x, orthoY.y, orthoZ.x, orthoZ.y);
}

bool LightDirect::NextDrawShadow(uint count, RenderTarget * rt)
{
    if (0 == count)
    {
        mPosition   = GetOwner()->GetTransform()->GetWorldPosition();
        auto up     = std::abs(mNormal.y) > 0.999f
                    ? glm::vec3(0, 0, 1) 
                    : glm::vec3(0, 1, 0);
        auto right  = glm::cross(up, mNormal);
        up          = glm::cross(mNormal, right);
        auto view   = glm::lookAt(mPosition, mPosition + mNormal, up);
        mMatrix     = _proj * view;

        glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
        auto base = glsl_tool::UBOAddData<decltype(UBOData::mSMP)>(0, mSMP);
        base = glsl_tool::UBOAddData<decltype(UBOData::mMatrix)>(base, mMatrix);
        base = glsl_tool::UBOAddData<decltype(UBOData::mNormal)>(base, mNormal);
        base = glsl_tool::UBOAddData<decltype(UBOData::mAmbient)>(base, mAmbient);
        base = glsl_tool::UBOAddData<decltype(UBOData::mDiffuse)>(base, mDiffuse);
        base = glsl_tool::UBOAddData<decltype(UBOData::mSpecular)>(base, mSpecular);
        base = glsl_tool::UBOAddData<decltype(UBOData::mPosition)>(base, mPosition);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glViewport(0, 0, Light::s_VIEW_W, Light::s_VIEW_H);
        Global::Ref().RefRender().GetMatrix().Identity(MatrixStack::kVIEW);
        Global::Ref().RefRender().GetMatrix().Identity(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrix().Mul(MatrixStack::kVIEW, view);
        Global::Ref().RefRender().GetMatrix().Mul(MatrixStack::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH,
                           RenderTarget::TextureType::k2D_ARRAY,
                           0, Light::s_lightPool.GetTex2D(), mSMP);
    }
    else
    {
        Global::Ref().RefRender().GetMatrix().Pop(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrix().Pop(MatrixStack::kVIEW);
    }
    return count == 0;
}

uint LightPoint::GetUBOLength()
{
    auto base = glsl_tool::UBOOffsetFill<decltype(UBOData::mSMP)>(0);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mFar)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mNear)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mK0)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mK1)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mK2)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mAmbient)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mDiffuse)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mSpecular)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mPosition)>(base);
    return glsl_tool::UBOOffsetBase<glm::vec4>(base);
}

void LightPoint::OpenShadow(const float n, const float f)
{
    glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
    glBufferData(GL_UNIFORM_BUFFER, GetUBOLength(), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    auto wdivh = (float)Light::s_VIEW_W / (float)Light::s_VIEW_W;
	_proj = glm::perspective(glm::radians(90.0f), wdivh, n, f);
    mFar = f; mNear = n;
}

bool LightPoint::NextDrawShadow(uint count, RenderTarget * rt)
{
    if (count != 0)
    {
        Global::Ref().RefRender().GetMatrix().Pop(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrix().Pop(MatrixStack::kVIEW);
    }
    else
    {
        glViewport(0, 0, Light::s_VIEW_W, Light::s_VIEW_H);

        mPosition = GetOwner()->GetTransform()->GetWorldPosition();

        glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
        auto base = glsl_tool::UBOAddData<decltype(UBOData::mSMP)>(0, mSMP);
        base = glsl_tool::UBOAddData<decltype(UBOData::mFar)>(base, mFar);
        base = glsl_tool::UBOAddData<decltype(UBOData::mNear)>(base, mNear);
        base = glsl_tool::UBOAddData<decltype(UBOData::mK0)>(base, mK0);
        base = glsl_tool::UBOAddData<decltype(UBOData::mK1)>(base, mK1);
        base = glsl_tool::UBOAddData<decltype(UBOData::mK2)>(base, mK2);
        base = glsl_tool::UBOAddData<decltype(UBOData::mAmbient)>(base, mAmbient);
        base = glsl_tool::UBOAddData<decltype(UBOData::mDiffuse)>(base, mDiffuse);
        base = glsl_tool::UBOAddData<decltype(UBOData::mSpecular)>(base, mSpecular);
        base = glsl_tool::UBOAddData<decltype(UBOData::mPosition)>(base, mPosition);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    if (count < 6)
    {
        static const std::tuple<glm::vec3, glm::vec3> s_faceInfo[6] = {
            { glm::vec3( 1,  0,  0), glm::vec3(0, -1,  0) },
            { glm::vec3(-1,  0,  0), glm::vec3(0, -1,  0) },
            { glm::vec3( 0,  1,  0), glm::vec3(0,  0,  1) },
            { glm::vec3( 0, -1,  0), glm::vec3(0,  0, -1) },
            { glm::vec3( 0,  0,  1), glm::vec3(0, -1,  0) },
            { glm::vec3( 0,  0, -1), glm::vec3(0, -1,  0) },
        };

        auto view = glm::lookAt(mPosition,
                    std::get<0>(s_faceInfo[count]) + mPosition,
                    std::get<1>(s_faceInfo[count]));

        Global::Ref().RefRender().GetMatrix().Identity(MatrixStack::kVIEW);
        Global::Ref().RefRender().GetMatrix().Identity(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrix().Mul(MatrixStack::kVIEW,  view);
        Global::Ref().RefRender().GetMatrix().Mul(MatrixStack::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH, 
                           RenderTarget::TextureType::k3D_ARRAY, 
                           count, Light::s_lightPool.GetTex3D(), mSMP);
    }
    return count != 6;
}

uint LightSpot::GetUBOLength()
{
    auto base = glsl_tool::UBOOffsetFill<decltype(UBOData::mSMP)>(0);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mK0)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mK1)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mK2)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mInCone)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mOutCone)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mMatrix)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mNormal)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mAmbient)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mDiffuse)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mSpecular)>(base);
    base = glsl_tool::UBOOffsetFill<decltype(UBOData::mPosition)>(base);
    return glsl_tool::UBOOffsetBase<glm::vec4>(base);
}

void LightSpot::OpenShadow(const float n, const float f)
{
    glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
    glBufferData(GL_UNIFORM_BUFFER, GetUBOLength(), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    auto wdivh = (float)Light::s_VIEW_W / (float)Light::s_VIEW_H;
    _proj = glm::perspective(glm::radians(90.0f), wdivh, n, f);
}

bool LightSpot::NextDrawShadow(uint count, RenderTarget * rt)
{
    if (count == 0)
    {
        mPosition   = GetOwner()->GetTransform()->GetWorldPosition();
        auto up     = std::abs(mNormal.y) > 0.999f
                    ? glm::vec3(0, 0, 1)
                    : glm::vec3(0, 1, 0);
        auto right  = glm::cross(up, mNormal);
        up          = glm::cross(mNormal, right);
        auto view   = glm::lookAt(mPosition, mPosition + mNormal, up);
        mMatrix     = _proj * view;

        glBindBuffer(GL_UNIFORM_BUFFER, GetUBO());
        auto base = glsl_tool::UBOAddData<decltype(UBOData::mSMP)>(0, mSMP);
        base = glsl_tool::UBOAddData<decltype(UBOData::mK0)>(base, mK0);
        base = glsl_tool::UBOAddData<decltype(UBOData::mK1)>(base, mK1);
        base = glsl_tool::UBOAddData<decltype(UBOData::mK2)>(base, mK2);
        base = glsl_tool::UBOAddData<decltype(UBOData::mInCone)>(base, mInCone);
        base = glsl_tool::UBOAddData<decltype(UBOData::mOutCone)>(base, mOutCone);
        base = glsl_tool::UBOAddData<decltype(UBOData::mMatrix)>(base, mMatrix);
        base = glsl_tool::UBOAddData<decltype(UBOData::mNormal)>(base, mNormal);
        base = glsl_tool::UBOAddData<decltype(UBOData::mAmbient)>(base, mAmbient);
        base = glsl_tool::UBOAddData<decltype(UBOData::mDiffuse)>(base, mDiffuse);
        base = glsl_tool::UBOAddData<decltype(UBOData::mSpecular)>(base, mSpecular);
        base = glsl_tool::UBOAddData<decltype(UBOData::mPosition)>(base, mPosition);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        Global::Ref().RefRender().GetMatrix().Identity(MatrixStack::kVIEW);
        Global::Ref().RefRender().GetMatrix().Identity(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrix().Mul(MatrixStack::kVIEW, view);
        Global::Ref().RefRender().GetMatrix().Mul(MatrixStack::kPROJ, _proj);
        rt->BindAttachment(RenderTarget::AttachmentType::kDEPTH,
                           RenderTarget::TextureType::k2D_ARRAY,
                           0, Light::s_lightPool.GetTex2D(), mSMP);
    }
    else
    {
        Global::Ref().RefRender().GetMatrix().Pop(MatrixStack::kPROJ);
        Global::Ref().RefRender().GetMatrix().Pop(MatrixStack::kVIEW);
    }
    return count == 0;
}

