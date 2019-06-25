#pragma once

enum RenderQueueEnum {
	kBACKGROUND,    //  �ײ����
	kGEOMETRIC,     //  �������
	kOPACITY,       //  ͸������
	kTOP,           //  �������
};

enum RenderTypeEnum {
    kSSAO,          //  SSAO
    kLIGHT,         //  ��Ⱦ��Դ
	kSHADOW,        //  ��Ӱ��ͼ
	kFORWARD,       //  ������Ⱦ
	kDEFERRED,      //  �ӳ���Ⱦ
};

enum DrawTypeEnum {
	kINSTANCE,		//	ʵ��draw
	kVERTEX,		//	����draw
	kINDEX,			//	����draw
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
//  ģ����ͼͶӰ����
static const char * const UNIFORM_MATRIX_MVP        = "matrix_mvp_";
//  ��Ϸʱ��
static const char * const UNIFORM_GAME_TIME         = "game_time_";

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
static const char * const UNIFORM_SCREEN_DEPTH         = "screen_depth_";

//  ������Դ-�ӳ���Ⱦ-�����
static const char * const BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_DIRECT = "res/built-in/mesh/deferred_light_volume_direct.obj";
static const char * const BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_POINT  = "res/built-in/mesh/deferred_light_volume_point.obj";
static const char * const BUILTIN_MESH_DEFERRED_LIGHT_VOLUME_SPOT   = "res/built-in/mesh/deferred_light_volume_spot.obj";
