#pragma once

#include "../include.h"
#include "../raw/gl_mesh.h"
#include "../raw/gl_program.h"
#include "../raw/gl_material.h"
#include "../raw/gl_texture2d.h"

enum RenderQueueEnum {
	kBACKGROUND,        //  �ײ����
	kGEOMETRIC,         //  �������
	kOPACITY,           //  ͸������
	kTOP,               //  �������
};

enum RenderTypeEnum {
    kLIGHT,             //  ��Ⱦ��Դ
    kDEPTH,             //  �����Ⱦ
    kSHADOW,            //  ��Ӱ��ͼ
	kFORWARD,           //  ������Ⱦ
	kDEFERRED,          //  �ӳ���Ⱦ
};

enum DrawTypeEnum {
	kINSTANCE,		    //	ʵ��draw
	kVERTEX,		    //	����draw
	kINDEX,			    //	����draw
};

enum FragTypeEnum {
    kTRIANGLE   = GL_TRIANGLES,
    kPOINT      = GL_POINTS,
    kLINE       = GL_LINES,
};

enum UniformBlockEnum {
    kLIGHT_DIRECT,      //  �����󶨵�
    kLIGHT_POINT,       //  ���Դ�󶨵�
    kLIGHT_SPOT,        //  �۹�ư󶨵�
};

//  UBO ���������
static const char * const UBO_NAME_LIGHT_DIRECT     = "LightDirect_";
//  UBO ���Դ����
static const char * const UBO_NAME_LIGHT_POINT      = "LightPoint_";
//  UBO �۹������
static const char * const UBO_NAME_LIGHT_SPOT       = "LightSpot_";

//  ��Դ����
static const char * const UNIFORM_LIGHT_TYPE        = "light_type_";
//  ��Դλ��
static const char * const UNIFORM_LIGHT_POS         = "light_pos_";
//  ���߾���
static const char * const UNIFORM_MATRIX_N          = "matrix_n_";
//  ģ�;���
static const char * const UNIFORM_MATRIX_M          = "matrix_m_";
//  ��ͼ����
static const char * const UNIFORM_MATRIX_V          = "matrix_v_";
//  ͶӰ����
static const char * const UNIFORM_MATRIX_P          = "matrix_p_";
//  ģ����ͼ����
static const char * const UNIFORM_MATRIX_MV         = "matrix_mv_";
//  ��ͼͶӰ����
static const char * const UNIFORM_MATRIX_VP         = "matrix_vp_";
//  ģ����ͼͶӰ����
static const char * const UNIFORM_MATRIX_MVP        = "matrix_mvp_";
//  ��ͼ����(��)
static const char * const UNIFORM_MATRIX_V_INV      = "matrix_v_inv_";
//  ͶӰ����(��)
static const char * const UNIFORM_MATRIX_P_INV      = "matrix_p_inv_";
//  ģ����ͼ����(��)
static const char * const UNIFORM_MATRIX_MV_INV     = "matrix_mv_inv_";
//  ��ͼͶӰ����(��)
static const char * const UNIFORM_MATRIX_VP_INV     = "matrix_vp_inv_";
//  ��Ϸʱ��
static const char * const UNIFORM_GAME_TIME         = "game_time_";

//  �ֱ��ʳߴ�
static const char * const UNIFORM_VIEW_SIZE         = "view_size_";
//  ������ü�����
static const char * const UNIFORM_CAMERA_N          = "camera_n_";
//  ���Զ�ü�����
static const char * const UNIFORM_CAMERA_F          = "camera_f_";
//  ���λ��
static const char * const UNIFORM_CAMERA_POS        = "camera_pos_";
//  �������
static const char * const UNIFORM_CAMERA_EYE        = "camera_eye_";

//  ����*
static const char * const UNIFORM_MATERIAL              = "material_.{0}";
//  �������Ӱ��ͼ
static const char * const UNIFORM_SHADOW_MAP_DIRECT_    = "shadow_map_direct_{0}_";
//  ���Դ��Ӱ��ͼ
static const char * const UNIFORM_SHADOW_MAP_POINT_     = "shadow_map_point_{0}_";
//  �۹����Ӱ��ͼ
static const char * const UNIFORM_SHADOW_MAP_SPOT_      = "shadow_map_spot_{0}_";
//  ���������
static const char * const UNIFORM_LIGHT_COUNT_DIRECT_   = "light_count_direct_";
//  ���Դ����
static const char * const UNIFORM_LIGHT_COUNT_POINT_    = "light_count_point_";
//  �۹������
static const char * const UNIFORM_LIGHT_COUNT_SPOT_     = "light_count_spot_";

//  G-Buffer Position
static const char * const UNIFORM_GBUFFER_POSIITON      = "gbuffer_position_";
static const char * const UNIFORM_GBUFFER_SPECULAR      = "gbuffer_specular_";
static const char * const UNIFORM_GBUFFER_DIFFUSE       = "gbuffer_diffuse_";
static const char * const UNIFORM_GBUFFER_NORMAL        = "gbuffer_normal_";

//  SSAO
static const char * const UNIFORM_SCREEN_POSTION        = "screen_position_";
static const char * const UNIFORM_SCREEN_DEPTH          = "screen_depth_";
static const char * const UNIFORM_SCREEN_SAO            = "screen_sao_";

//  ������Դ-�ӳ���Ⱦ-�����
static const char * const BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_DIRECT = "res/built-in/mesh/deferred_light_volume_direct.obj";
static const char * const BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_POINT  = "res/built-in/mesh/deferred_light_volume_point.obj";
static const char * const BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_SPOT   = "res/built-in/mesh/deferred_light_volume_spot.obj";
static const char * const BUILTIN_MESH_SCREEN_QUAD                  = "res/built-in/mesh/screen_quad.obj";
//  SSAO ��ɫ��
static const char * const BUILTIN_PROGRAM_SSAO                      = "res/built-in/program/ssao.program";

//  ����ջ
class MatrixStack {
public:
    enum ModeType { kPROJ, kVIEW, kMODEL, };

public:
    MatrixStack()
    { }

    ~MatrixStack()
    { }

    void Pop(ModeType mode)
    {
        GetStack(mode).pop();
    }

    void Push(ModeType mode)
    {
        GetStack(mode).push(GetStack(mode).top());
    }

    void Identity(ModeType mode)
    {
        GetStack(mode).push(glm::mat4(1));
    }

    void Mul(ModeType mode, const glm::mat4 & mat)
    {
        GetStack(mode).top() *= mat;
    }

    const glm::mat4 & Top(ModeType mode) const
    {
        return GetStack(mode).top();
    }

    const glm::mat4 & GetM() const
    {
        return Top(ModeType::kMODEL);
    }

    const glm::mat4 & GetV() const
    {
        return Top(ModeType::kVIEW);
    }

    const glm::mat4 & GetP() const
    {
        return Top(ModeType::kPROJ);
    }

private:
    std::stack<glm::mat4> & GetStack(ModeType mode) const
    {
        return _matrixs.at((size_t)mode);
    }

private:
    mutable std::array<std::stack<glm::mat4>, 3> _matrixs;
};

enum class CommandEnum {
    kMATERIAL,
    kCAMERA,
    kLIGHT,
};
//  ������Ⱦ������ṹ
struct RenderCommand {
};

//  ������Ⱦ����
struct MaterialCommand : public RenderCommand {
    MaterialCommand() { }
    const GLMaterial *  mMaterial;
    glm::mat4           mTransform;
    uint                mCameraMask;
    uint                mSubPass;
};

//  �����Ⱦ����
struct CameraCommand : public RenderCommand {
    CameraCommand() { }
    glm::vec4   mViewport;
    glm::mat4   mProj;
    glm::mat4   mView;
    glm::vec3   mPos;
    glm::vec3   mEye;
    float       mN, mF;
    uint        mOrder;
    uint        mMask;

    bool operator<(const CameraCommand & other) const
    {
        return mOrder < other.mOrder;
    }
};

//  ��Դ��Ⱦ����
struct LightCommand : public RenderCommand {
    LightCommand() { }
    glm::vec3   mPosition;
    glm::mat4   mTransform;
    glm::mat4   mView;
    glm::mat4   mProj;
    uint        mType;
    uint        mUBO;
    GLMesh    * mMesh;
    GLProgram * mProgram;
};
