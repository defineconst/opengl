#pragma once

enum RenderQueueEnum {
	kBACKGROUND,    //  底层绘制
	kGEOMETRIC,     //  常规绘制
	kOPACITY,       //  透明绘制
	kTOP,           //  顶层绘制
};

enum RenderTypeEnum {
	kSHADOW,        //  烘培阴影贴图
	kFORWARD,       //  正向渲染
	kDEFERRED,      //  延迟渲染
};

enum DrawTypeEnum {
	kINSTANCE,		//	实例draw
	kVERTEX,		//	顶点draw
	kINDEX,			//	索引draw
};

enum UniformBlockEnum {
    kLIGHT_DIRECT,      //  方向光绑定点
    kLIGHT_POINT,       //  点光源绑定点
    kLIGHT_SPOT,        //  聚光灯绑定点
};

//  UBO 方向光数据
static const char * const UBO_NAME_LIGHT_DIRECT = "LightDirect_";
//  UBO 点光源数据
static const char * const UBO_NAME_LIGHT_POINT = "LightPoint_";
//  UBO 聚光灯数据
static const char * const UBO_NAME_LIGHT_SPOT = "LightSpot_";

//  光源位置
static const char * const UNIFORM_LIGHT_POS = "light_pos_";
//  光源类型
static const char * const UNIFORM_LIGHT_TYPE = "light_type_";
//  法线矩阵
static const char * const UNIFORM_MATRIX_N = "matrix_n_";
//  模型矩阵
static const char * const UNIFORM_MATRIX_M = "matrix_m_";
//  视图矩阵
static const char * const UNIFORM_MATRIX_V = "matrix_v_";
//  投影矩阵
static const char * const UNIFORM_MATRIX_P = "matrix_p_";
//  模型视图矩阵
static const char * const UNIFORM_MATRIX_MV = "matrix_mv_";
//  模型视图投影矩阵
static const char * const UNIFORM_MATRIX_MVP = "matrix_mvp_";
//  游戏时间
static const char * const UNIFORM_GAME_TIME = "game_time_";

//  相机位置
static const char * const UNIFORM_CAMERA_POS = "camera_pos_";
//  相机朝向
static const char * const UNIFORM_CAMERA_EYE = "camera_eye_";

//  材质.漫反射
static const char * const UNIFORM_MATERIAL_DIFFUSE      = "material_.mDiffuse{0}";
//  材质.镜面反射
static const char * const UNIFORM_MATERIAL_SPECULAR     = "material_.mSpecular";
//  材质.反射贴图
static const char * const UNIFORM_MATERIAL_REFLECT      = "material_.mReflect";
//  材质.法线贴图
static const char * const UNIFORM_MATERIAL_NORMAL       = "material_.mNormal";
//  材质.高度贴图
static const char * const UNIFORM_MATERIAL_HEIGHT       = "material_.mHeight";
//  材质.高光强度
static const char * const UNIFORM_MATERIAL_SHININESS    = "material_.mShininess";
//  2D阴影贴图(方向光, 聚光灯)
static const char * const UNIFORM_SHADOW_MAP_2D_        = "shadow_map_2d_";
//  3D阴影贴图(点光源)
static const char * const UNIFORM_SHADOW_MAP_3D_        = "shadow_map_3d_";
//  方向光数量
static const char * const UNIFORM_LIGHT_COUNT_DIRECT_   = "light_count_direct_";
//  点光源数量
static const char * const UNIFORM_LIGHT_COUNT_POINT_    = "light_count_point_";
//  聚光灯数量
static const char * const UNIFORM_LIGHT_COUNT_SPOT_     = "light_count_spot_";
