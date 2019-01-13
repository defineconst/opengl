#pragma once

#include "../include.h"

class Mesh;
class Camera;
class Shader;
class Material;

class Render {
public:
	enum GL_UBO_IDX {
		kLIGHT,
	};

	class Matrix {
	public:
		enum ModeType { kPROJECT, kMODELVIEW, };

	public:
		Matrix()
		{ }

		~Matrix()
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
			auto & stack = GetStack(mode);
			if (stack.empty())
			{ stack.push(glm::mat4(1)); }
			else
			{ stack.top() = glm::mat4(1); }
		}

		void Mul(ModeType mode, const glm::mat4 & mat)
		{
			GetStack(mode).top() *= mat;
		}

		const glm::mat4 & Top(ModeType mode) const
		{
			return GetStack(mode).top();
		}

		glm::mat4 GetNMat() const
		{
			return glm::transpose(glm::inverse(GetMV()));
		}

		glm::mat4 GetMVP() const
		{
			return _project.top() * _modelview.top();
		}

		const glm::mat4 & GetMV() const
		{
			return Top(ModeType::kMODELVIEW);
		}

		const glm::mat4 & GetP() const
		{
			return Top(ModeType::kPROJECT);
		}

	private:
		std::stack<glm::mat4> & GetStack(ModeType mode)
		{
			return ModeType::kPROJECT == mode ? _project : _modelview;
		}

		const std::stack<glm::mat4> & GetStack(ModeType mode) const
		{
			return const_cast<Matrix *>(this)->GetStack(mode);
		}

	private:
		std::stack<glm::mat4> _project;
		std::stack<glm::mat4> _modelview;
	};

    struct Camera {
		size_t mID;
		::Camera * mCamera;
        Camera(): mCamera(nullptr), mID(0) { }
        Camera(::Camera * camera, size_t id)
			: mCamera(camera), mID(id) { }
        bool operator ==(size_t id) const { return mID == id; }
        bool operator <(size_t id) const { return mID < id; }
    };

	struct RenderInfo {
		Material * mMaterial;
		::Camera * mCamera;
		Shader * mShader;
		Mesh * mMesh;
	};

    //  �任����
    struct CommandTransform {
		static void Post(size_t cameraID, const glm::mat4 & mat);
		static void Free(size_t cameraID);
    };

    struct Command {
        size_t mCameraID;
        std::function<void()> mCallFn;
        Command(): mCameraID(0) { }
    };

public:
    Render();
    ~Render();

	Matrix & GetMatrix();

	void Bind(Mesh * mesh);
	void Bind(Shader * shader);
	void Bind(::Camera * camera);
	void Bind(Material * material);

    void AddCamera(size_t id, ::Camera * camera);
    void DelCamera(size_t id);
    
	void RenderMesh(size_t count);
	void RenderOnce();

	void PostCommand(const Command & command);

private:
	void OnRenderCamera(Camera & camera);

private:
	Matrix _matrix;
	RenderInfo _renderInfo;
    std::vector<Camera> _cameras;
    std::vector<Command> _commands;
};