#pragma once

#include "../include.h"

class Camera;
class Shader;

class Render {
public:
	//  ��Ⱦ����
	enum QueueType {
		//  ����
		kBACKGROUND,
		//  ����
		kGEOMETRY,
		//  ͸��
		kOPATCIY,
		//  ����
		kOVERLAY,
		MAX,
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

	private:
		std::stack<glm::mat4> & GetStack(ModeType mode)
		{
			return ModeType::kPROJECT == mode
				? _project : _modelview;
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

    //  �任����
    struct CommandTransform {
		static void Push(const glm::mat4 & mat);
		static void Pop();
    };

    struct Command {
        size_t mCameraID;
        std::function<void()> mCallFn;
        Command(): mCameraID(0) { }
    };

public:
    Render();
    ~Render();

    void AddCamera(::Camera * camera, size_t id);
    void DelCamera(size_t id);
    
	void PostCommand(QueueType queue, const Command & command);
	
	Matrix & GetMatrix();
    
	void RenderOnce();

private:
	void OnRenderCamera(Camera & camera);

private:
	Matrix _matrix;

    std::vector<Camera> _cameras;

    std::vector<std::vector<Command>> _renderQueue;
};