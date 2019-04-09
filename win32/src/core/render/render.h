#pragma once

#include "../include.h"
#include "render_type.h"

class RenderTarget;

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
        size_t mTexCount;
		size_t mVertexCount;
		size_t mRenderCount;
        const RenderPass * mPass;
		RenderInfo()
			: mTexCount(0)
			, mVertexCount(0)
			, mRenderCount(0)
			, mPass(nullptr)
		{ }
	};

public:
    Render();
    ~Render();

	RenderMatrix & GetMatrix();

    //  相机
    void AddCamera(Camera * camera, size_t flag, size_t order = ~0);
	Camera * GetCamera(size_t order);
	void DelCamera(Camera * camera);
	void DelCamera(size_t order);

    //  光源
	void AddLight(Light * light);
	void DelLight(Light * light);
    
    //  渲染
	void RenderOnce();

	void PostCommand(const Shader * shader, const RenderCommand & command);

    //  辅助
	const RenderInfo & GetRenderInfo() const { return _renderInfo; }

private:
    void Bind(Light * light);
    void Bind(CameraInfo * camera);
    bool Bind(const RenderPass * pass);
    void Bind(const Material * material);

    //	执行绘制命令
    void Draw(DrawTypeEnum drawType, const RenderMesh & mesh);

    void ClearCommands();

    //  生成ShadowMap
	void OnRenderShadow(Light * light);

    //  逐相机渲染
    void OnRenderCamera();
    void OnRenderForward(CameraInfo * camera);
    void OnRenderDeferred(CameraInfo * camera);
	void OnRenderForwardCommands(CameraInfo * camera, Light * light, const RenderQueue & commands);
	void OnRenderDeferredCommands(CameraInfo * camera, Light * light, const RenderQueue & commands);
	
	//	绑定光照参数到着色器
	void BindLightParam();
	//	绑定每一帧渲染都可能变化的参数
	void BindFrameParam(CameraInfo * camera, Light * light);
	//	绑定每一次渲染都可能变化的参数
	void BindEveryParam(CameraInfo * camera, Light * light, const RenderCommand & command);

private:
    RenderMatrix    _matrix;
    RenderTarget *  _shadowRT;
	RenderInfo      _renderInfo;

	std::vector<Light *>    _lights;
    std::vector<CameraInfo> _cameraInfos;

	//	渲染队列, 阴影不需要区分队列类型
    RenderQueue _shadowCommands;
    std::array<RenderQueue, 4> _forwardCommands;
    std::array<RenderQueue, 4> _deferredCommands;
};