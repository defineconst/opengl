#pragma once

#include "../include.h"

class Light;
class Camera;
class Shader;
class Bitmap;
class Material;
class BitmapCube;

enum RenderQueueEnum {
	kBACKGROUND,    //  �ײ����
	kGEOMETRIC,     //  �������
	kOPACITY,       //  ͸������
	kTOP,           //  �������
};

enum RenderTypeEnum {
	kSHADOW,        //  ������Ӱ��ͼ
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
static const char * const UBO_NAME_LIGHT_DIRECT = "LightDirect_";
//  UBO ���Դ����
static const char * const UBO_NAME_LIGHT_POINT = "LightPoint_";
//  UBO �۹������
static const char * const UBO_NAME_LIGHT_SPOT = "LightSpot_";

//  ��Դλ��
static const char * const UNIFORM_LIGHT_POS = "light_pos_";
//  ��Դ����
static const char * const UNIFORM_LIGHT_TYPE = "light_type_";
//  ���߾���
static const char * const UNIFORM_MATRIX_N = "matrix_n_";
//  ģ�;���
static const char * const UNIFORM_MATRIX_M = "matrix_m_";
//  ��ͼ����
static const char * const UNIFORM_MATRIX_V = "matrix_v_";
//  ͶӰ����
static const char * const UNIFORM_MATRIX_P = "matrix_p_";
//  ģ����ͼ����
static const char * const UNIFORM_MATRIX_MV = "matrix_mv_";
//  ģ����ͼͶӰ����
static const char * const UNIFORM_MATRIX_MVP = "matrix_mvp_";
//  ��Ϸʱ��
static const char * const UNIFORM_GAME_TIME = "game_time_";

//  ���λ��
static const char * const UNIFORM_CAMERA_POS = "camera_pos_";
//  �������
static const char * const UNIFORM_CAMERA_EYE = "camera_eye_";

//  ����.������
static const char * const UNIFORM_MATERIAL_DIFFUSE      = "material_.mDiffuse{0}";
//  ����.���淴��
static const char * const UNIFORM_MATERIAL_SPECULAR     = "material_.mSpecular";
//  ����.������ͼ
static const char * const UNIFORM_MATERIAL_REFLECT      = "material_.mReflect";
//  ����.������ͼ
static const char * const UNIFORM_MATERIAL_NORMAL       = "material_.mNormal";
//  ����.�߶���ͼ
static const char * const UNIFORM_MATERIAL_HEIGHT       = "material_.mHeight";
//  ����.�߹�ǿ��
static const char * const UNIFORM_MATERIAL_SHININESS    = "material_.mShininess";
//  2D��Ӱ��ͼ(�����, �۹��)
static const char * const UNIFORM_SHADOW_MAP_2D_        = "shadow_map_2d_";
//  3D��Ӱ��ͼ(���Դ)
static const char * const UNIFORM_SHADOW_MAP_3D_        = "shadow_map_3d_";
//  ���������
static const char * const UNIFORM_LIGHT_COUNT_DIRECT_   = "light_count_direct_";
//  ���Դ����
static const char * const UNIFORM_LIGHT_COUNT_POINT_    = "light_count_point_";
//  �۹������
static const char * const UNIFORM_LIGHT_COUNT_SPOT_     = "light_count_spot_";

class RenderMatrix {
public:
	enum ModeType { kPROJ, kVIEW, kMODEL, };

public:
	RenderMatrix()
	{ }

	~RenderMatrix()
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
	std::stack<glm::mat4> & GetStack(ModeType mode)
	{
		return _RenderMatrixs.at((size_t)mode);
	}

	const std::stack<glm::mat4> & GetStack(ModeType mode) const
	{
		return const_cast<RenderMatrix *>(this)->GetStack(mode);
	}

private:
	std::array<std::stack<glm::mat4>, 3> _RenderMatrixs;
};

struct RenderMesh {

	struct Vertex {
		glm::vec3 v;
		struct {
			float u;
			float v;
		} uv; 
		glm::vec3 n;
		glm::vec3 tan;
		glm::vec3 bitan;

		Vertex(const glm::vec3 & _v, 
			   float _tx, float _ty, 
			   const glm::vec3 & _n, 
			   const glm::vec3 & _tan, 
			   const glm::vec3 & _bitan) : v(_v), uv{ _tx, _ty }, n(_n), tan(_tan), bitan(_bitan)
		{ }

		Vertex(const glm::vec3 & _v,
			   float _tx, float _ty,
			   const glm::vec3 & _n) : v(_v), uv{ _tx, _ty }, n(_n)
		{ }

		Vertex(const glm::vec3 & _v,
			   float _tx, float _ty) : v(_v), uv{ _tx, _ty }
		{ }

		Vertex(const glm::vec3 & _v) : v(_v)
		{ }

		Vertex() 
		{ }
	};

	uint mVAO, mVBO, mEBO;
	
	uint mVtxCount, mIdxCount;

	RenderMesh() 
		: mVAO(0), mVBO(0), mEBO(0)
		, mVtxCount(0), mIdxCount(0)
	{ }

	//	��������
	//		�������꣬�������꣬���ߣ����ߣ�������
	static RenderMesh Create(const std::vector<Vertex> & vertexs)
	{
		RenderMesh mesh;
		mesh.mVtxCount = static_cast<uint>(vertexs.size());
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, n));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tan));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, bitan));
		glEnableVertexAttribArray(4);

		glBindVertexArray(0);
		return mesh;
	}

	//	��������
	//		�������꣬�������꣬���ߣ����ߣ�������
	static RenderMesh Create(const std::vector<Vertex> & vertexs, const std::vector<uint> & indexs)
	{
		RenderMesh mesh;
		mesh.mIdxCount = static_cast<uint>(indexs.size());
		mesh.mVtxCount = static_cast<uint>(vertexs.size());
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &mesh.mEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(uint), indexs.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, n));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tan));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, bitan));
		glEnableVertexAttribArray(4);

		glBindVertexArray(0);
		return mesh;
	}

	//	��������
	//		�������꣬�������꣬����
	static RenderMesh CreateVTN(const std::vector<Vertex> & vertexs, const std::vector<uint> & indexs)
	{
		RenderMesh mesh;
		mesh.mIdxCount = static_cast<uint>(indexs.size());
		mesh.mVtxCount = static_cast<uint>(vertexs.size());
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &mesh.mEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(uint), indexs.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, n));
		glEnableVertexAttribArray(2);

		glBindVertexArray(0);
		return mesh;
	}

	//	��������
	//		�������꣬��������
	static RenderMesh CreateVT(const std::vector<Vertex> & vertexs, const std::vector<uint> & indexs)
	{
		RenderMesh mesh;
		mesh.mIdxCount = static_cast<uint>(indexs.size());
		mesh.mVtxCount = static_cast<uint>(vertexs.size());
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &mesh.mEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(uint), indexs.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
		return mesh;
	}

	//	��������
	//		��������
	static RenderMesh CreateV(const std::vector<Vertex> & vertexs, const std::vector<uint> & indexs)
	{
		RenderMesh mesh;
		mesh.mIdxCount = static_cast<uint>(indexs.size());
		mesh.mVtxCount = static_cast<uint>(vertexs.size());
		glGenVertexArrays(1, &mesh.mVAO);
		glBindVertexArray(mesh.mVAO);

		glGenBuffers(1, &mesh.mVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.mVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(Vertex), vertexs.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &mesh.mEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.mEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(uint), indexs.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		return mesh;
	}


	static void Delete(RenderMesh & mesh)
	{
		glDeleteBuffers(1, &mesh.mEBO);
		glDeleteBuffers(1, &mesh.mVBO);
		glDeleteVertexArrays(1, &mesh.mVAO);
		mesh.mVtxCount = mesh.mIdxCount = mesh.mVAO = mesh.mVBO = mesh.mEBO = 0;
	}
};


struct RenderPass {
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

    RenderPass() : GLID(0), bCullFace(false), bBlend(false), bDepthTest(false), bStencilTest(false)
    { }
};

//  ������Ⱦ������ṹ
struct RenderCommand {
    const RenderPass *  mPass;          //  �󶨵�Shader
    const RenderMesh *  mMeshs;         //  �󶨵�����
	uint                mMeshNum;       //  �󶨵���������
	const Material *    mMaterials;     //  �󶨵Ĳ���(������������������һ��)
	glm::mat4           mTransform;     //  �󶨵ı任����
    uint                mCameraFlag;    //  �󶨵������ʶ
};

using RenderQueue = std::vector<RenderCommand>;