#pragma once

#include "../include.h"
#include "render_type.h"

class Render {
public:
    struct CameraInfo {
		enum Flag {
			kFLAG0 = 0x1,	kFLAG1 = 0x2,	kFLAG2 = 0x4,	kFLAG3 = 0x8,
			kFLAG4 = 0x10,	kFLAG5 = 0x20,	kFLAG6 = 0x30,	kFLAG7 = 0x40,
		};

		size_t mFlag;
		size_t mOrder;
		Camera * mCamera;
        CameraInfo(): mCamera(nullptr), mOrder(0), mFlag(0) { }
        CameraInfo(Camera * camera, size_t flag, size_t order) 
			: mCamera(camera), mFlag(flag), mOrder(order) { }
    };

	struct RenderInfo {
        Camera * mCamera;
        size_t mTexCount;
		size_t mVertexCount;
		size_t mRenderCount;
		const RenderPass * mPass;
		RenderInfo()
			: mTexCount(0)
			, mVertexCount(0)
			, mRenderCount(0)
            , mCamera(nullptr)
			, mPass(nullptr)
		{ }
	};

public:
    Render();
    ~Render();

	RenderMatrix & GetMatrix();

    void AddCamera(Camera * camera, size_t flag, size_t order = ~0);
	Camera * GetCamera(size_t order);
	void DelCamera(Camera * camera);
	void DelCamera(size_t order);

	void AddLight(Light * light);
	void DelLight(Light * light);
    
	void OnRenderCamera(CameraInfo * camera);
	void RenderOnce();

	void PostCommand(const Shader * shader, const RenderCommand & command);

	const RenderInfo & GetRenderInfo() const { return _renderInfo; }

private:
    void OnRenderForward(CameraInfo * camera);
    void OnRenderDeferred(CameraInfo * camera);
	void OnRenderForwardCommands(CameraInfo * camera, const RenderQueue & commands);
	void OnRenderDeferredCommands(CameraInfo * camera, const RenderQueue & commands);

	//	��������ɫ��
	void BindTexture(const std::string & key, const Bitmap * val);
	void BindTexture(const std::string & key, const Texture & val);
	void BindTexture(const std::string & key, const BitmapCube * val);

	void Bind(Camera * camera);
	bool Bind(const RenderPass & pass);
	void Bind(const Material & material);

	//	�󶨹��ղ�������ɫ��
	void BindLightParam();
	//	��ÿһ֡��Ⱦ�����ܱ仯�Ĳ���
	void BindFrameParam();
	//	��ÿһ����Ⱦ�����ܱ仯�Ĳ���
	void BindEveryParam(const RenderCommand & command);
	//	ִ�л�������
	void Draw(DrawTypeEnum drawType, const Mesh & mesh);

private:
    RenderMatrix _matrix;
	RenderInfo _renderInfo;
	std::vector<Light *> _lights;
    std::vector<CameraInfo> _cameraInfos;
    std::array<RenderQueue, 4> _shadowCommands;
    std::array<RenderQueue, 4> _forwardCommands;
    std::array<RenderQueue, 4> _deferredCommands;
};