#include "renderer.h"
#include "pipe.h"
#include "../component/light.h"

Renderer::Renderer(): _state(nullptr)
{ }

Renderer::~Renderer()
{
    while (!_pipes.empty())
    {
        delete _pipes.back();
        _pipes.pop_back();
    }
    SAFE_DELETE(_state);
}

MatrixStack & Renderer::GetMaterialStack()
{
    return _state->mMaterialStack;
}

uint Renderer::GetVertexCount()
{
    return _state->mRenderTime.mVertexCount;
}

uint Renderer::GetDrawCount()
{
    return _state->mRenderTime.mDrawCount;
}

void Renderer::RenderOnce()
{
    for (auto & camera : _state->mCameraQueue)
    {
        Bind(&camera);
        for (auto & pipe : _pipes)
        {pipe->OnUpdate(this, _state);}
        Bind((CameraCommand *)nullptr);
    }
}

void Renderer::AddPipe(Pipe * pipe)
{
    _pipes.push_back(pipe);
    pipe->OnAdd(this, _state);
}

void Renderer::Bind(const CameraCommand * command)
{
    if (command != nullptr)
    {
        _state->mMaterialStack.Identity(MatrixStack::kVIEW);
        _state->mMaterialStack.Identity(MatrixStack::kPROJ);
        _state->mMaterialStack.Mul(MatrixStack::kVIEW, command->mView);
        _state->mMaterialStack.Mul(MatrixStack::kPROJ, command->mProj);
        glViewport((int)command->mViewport.x, (int)command->mViewport.y,
                   (int)command->mViewport.z, (int)command->mViewport.w);
        _state->mRenderTime.mCamera = command;
    }
    else
    {
        _state->mMaterialStack.Pop(MatrixStack::kVIEW);
        _state->mMaterialStack.Pop(MatrixStack::kPROJ);
        _state->mRenderTime.mCamera = nullptr;
    }
}

bool Renderer::Bind(const GLProgram * program, uint pass)
{
    if (_state->mRenderTime.mProgram != program)
    {
        _state->mRenderTime.mTexBase = 0;
        _state->mRenderTime.mProgram = program;
        _state->mRenderTime.mProgram->UsePass(pass, true);
        return true;
    }
    return _state->mRenderTime.mProgram->UsePass(pass);
}

void Renderer::Post(const Light * light)
{
    switch (light->GetType())
    {
    case Light::kDIRECT:
        {
            auto idx = glGetUniformBlockIndex(_state->mRenderTime.mProgram->GetUseID(), UBO_NAME_LIGHT_DIRECT);
            if (GL_INVALID_INDEX != idx)
            {
                glUniformBlockBinding(_state->mRenderTime.mProgram->GetUseID(), idx, UniformBlockEnum::kLIGHT_DIRECT);
                glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_DIRECT, light->GetUBO());
            }
        }
        break;
    case Light::kPOINT:
        {
            auto idx = glGetUniformBlockIndex(_state->mRenderTime.mProgram->GetUseID(), UBO_NAME_LIGHT_POINT);
            if (GL_INVALID_INDEX != idx)
            {
                glUniformBlockBinding(_state->mRenderTime.mProgram->GetUseID(), idx, UniformBlockEnum::kLIGHT_POINT);
                glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_POINT, light->GetUBO());
            }
        }
        break;
    case Light::kSPOT:
        {
            auto idx = glGetUniformBlockIndex(_state->mRenderTime.mProgram->GetUseID(), UBO_NAME_LIGHT_SPOT);
            if (GL_INVALID_INDEX != idx)
            {
                glUniformBlockBinding(_state->mRenderTime.mProgram->GetUseID(), idx, UniformBlockEnum::kLIGHT_SPOT);
                glBindBufferBase(GL_UNIFORM_BUFFER, UniformBlockEnum::kLIGHT_SPOT, light->GetUBO());
            }
        }
        break;
    }
    _state->mRenderTime.mProgram->BindUniformNumber(UNIFORM_LIGHT_TYPE, light->GetType());
}

void Renderer::Post(const glm::mat4 * model)
{
    const auto & matrixM    = *model;
    const auto & matrixV    = _state->mMaterialStack.GetV();
    const auto & matrixP    = _state->mMaterialStack.GetP();
    const auto & matrixN    = glm::transpose(glm::inverse(glm::mat3(matrixM)));
    const auto & matrixMV   = matrixV * matrixM;
    const auto & matrixVP   = matrixP * matrixV;
    const auto & matrixMVP  = matrixP * matrixMV;

    //  ����
    _state->mRenderTime.mProgram->BindUniformMatrix(UNIFORM_MATRIX_N, matrixN);
    _state->mRenderTime.mProgram->BindUniformMatrix(UNIFORM_MATRIX_M, matrixM);
    _state->mRenderTime.mProgram->BindUniformMatrix(UNIFORM_MATRIX_V, matrixV);
    _state->mRenderTime.mProgram->BindUniformMatrix(UNIFORM_MATRIX_P, matrixP);
    _state->mRenderTime.mProgram->BindUniformMatrix(UNIFORM_MATRIX_MV, matrixMV);
    _state->mRenderTime.mProgram->BindUniformMatrix(UNIFORM_MATRIX_VP, matrixVP);
    _state->mRenderTime.mProgram->BindUniformMatrix(UNIFORM_MATRIX_MVP, matrixMVP);

    //  �����
    _state->mRenderTime.mProgram->BindUniformMatrix(UNIFORM_MATRIX_V_INV, glm::inverse(matrixV));
    _state->mRenderTime.mProgram->BindUniformMatrix(UNIFORM_MATRIX_P_INV, glm::inverse(matrixP));
    _state->mRenderTime.mProgram->BindUniformMatrix(UNIFORM_MATRIX_MV_INV, glm::inverse(matrixMV));
    _state->mRenderTime.mProgram->BindUniformMatrix(UNIFORM_MATRIX_VP_INV, glm::inverse(matrixVP));

    //  ��������
    _state->mRenderTime.mProgram->BindUniformNumber(UNIFORM_GAME_TIME, glfwGetTime());
    if (_state->mRenderTime.mCamera != nullptr)
    {
        _state->mRenderTime.mProgram->BindUniformNumber(UNIFORM_CAMERA_N, _state->mRenderTime.mCamera->mN);
        _state->mRenderTime.mProgram->BindUniformNumber(UNIFORM_CAMERA_F, _state->mRenderTime.mCamera->mF);
        _state->mRenderTime.mProgram->BindUniformVector(UNIFORM_CAMERA_POS, _state->mRenderTime.mCamera->mPos);
        _state->mRenderTime.mProgram->BindUniformVector(UNIFORM_CAMERA_EYE, _state->mRenderTime.mCamera->mEye);
        _state->mRenderTime.mProgram->BindUniformVector(UNIFORM_VIEW_SIZE, glm::vec2(
            _state->mRenderTime.mCamera->mViewport.z - _state->mRenderTime.mCamera->mViewport.x,
            _state->mRenderTime.mCamera->mViewport.w - _state->mRenderTime.mCamera->mViewport.y));
    }
}

void Renderer::Post(const GLMaterial * material)
{
    for (auto i = 0; i != material->GetItems().size(); ++i)
    {
        const auto & item = material->GetItems().at(i);
        const auto & key  = SFormat(UNIFORM_MATERIAL, item.mKey);
        if (item.mTex2D != nullptr)
        {
            _state->mRenderTime.mProgram->BindUniformTex2D(key.c_str(), item.mTex2D->GetID(), 
                                                           _state->mRenderTime.mTexBase + i);
        }
    }
}

void Renderer::Post(const CommandEnum type, const RenderCommand * command)
{
    switch (type)
    {
    case CommandEnum::kMATERIAL:
        {
            auto cmd = (MaterialCommand &)command;
            auto pro = cmd.mMaterial->GetProgram();
            for (auto it = pro->GetPasss().begin();
                 it != pro->GetPasss().end(); ++it)
            {
                cmd.mSubPass = std::distance(pro->GetPasss().begin(), it);
                switch (it->mRenderType)
                {
                case RenderTypeEnum::kDEPTH:        _state->mDepthQueue.push_back(cmd);                             break;
                case RenderTypeEnum::kSHADOW:       _state->mShadowQueue.push_back(cmd);                            break;
                case RenderTypeEnum::kFORWARD:      _state->mForwardQueues.at(it->mRenderQueue).push_back(cmd);     break;
                case RenderTypeEnum::kDEFERRED:     _state->mDeferredQueues.at(it->mRenderQueue).push_back(cmd);    break;
                }
            }
        }
        break;
    case CommandEnum::kCAMERA:
        {
            _state->mCameraQueue.push_back((const CameraCommand &)command);
        }
        break;
    case CommandEnum::kLIGHT:
        {
            const auto & cmd = (const LightCommand &)command;
            _state->mLightQueues.at(cmd.mLight->GetType()).push_back(cmd);
        }
        break;
    }
}

void Renderer::Post(const DrawTypeEnum type, const GLMesh * mesh)
{
    ASSERT_LOG(mesh->GetVAO() != 0, "Draw VAO Error");

    glBindVertexArray(mesh->GetVAO());
    switch (type)
    {
    case DrawTypeEnum::kINSTANCE:
        {
            //	TODO, �ݲ�ʵ��
        }
        break;
    case DrawTypeEnum::kVERTEX:
        {
            _state->mRenderTime.mVertexCount += mesh->GetVCount();
            glDrawArrays(GL_TRIANGLES, 0, mesh->GetVCount());
        }
        break;
    case DrawTypeEnum::kINDEX:
        {
            _state->mRenderTime.mVertexCount += mesh->GetECount();
            glDrawElements(GL_TRIANGLES, mesh->GetECount(), GL_UNSIGNED_INT, nullptr);
        }
        break;
    }
    ++_state->mRenderTime.mDrawCount;
}
