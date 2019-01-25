#include "core/mmc.h"
#include "core/timer/timer.h"
#include "core/event/event.h"
#include "core/window/window.h"
#include "core/object/camera.h"
#include "core/render/render.h"
#include "core/asset/shader.h"
#include "core/asset/material.h"
#include "core/component/sprite.h"
#include "core/component/sprite_outline.h"
#include "core/asset/asset_core.h"
#include "core/tools/debug_tool.h"
#include "core/component/light.h"
#include "core/component/transform.h"
#include "core/asset/file.h"

class AppWindow : public Window {
public:
	const float s_MAX_SPEED = 0.1f;
	
	enum DirectEnum {
		kNONE,
		kFRONT = 1,
		kBACK = 2,
		kLEFT = 4,
		kRIGHT = 8,
		kUP = 16,
		kDOWN = 32,
	};

public:
    void InitGame()
    {
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		InitCamera();
		InitAssets();
		InitEvents();
		InitLights();
		InitObject();
    }
private:
	void InitCamera()
	{
		auto camera = new Camera();
		camera->Init(60, (float)GetW(), (float)GetH(), 0.1f, 500);
		camera->LookAt(
			glm::vec3(0, 3, 3),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0));
		mmc::mRender.AddCamera(0, camera);
	}

	void InitAssets()
	{
		File::LoadShader("res/shader/outline/normal.shader");
		File::LoadShader("res/shader/outline/outline.shader");
		auto box = File::LoadModel("res/model/outline/box.obj");
		auto floor = File::LoadModel("res/model/outline/floor.obj");
		auto boxTex = File::LoadTexture("res/model/outline/box.jpg");
		auto floorTex = File::LoadTexture("res/model/outline/floor.png");
		box->mChilds.at(0)->mMaterials.at(0).mDiffuses.push_back(boxTex);
		floor->mChilds.at(0)->mMaterials.at(0).mDiffuses.push_back(floorTex);
	}

	void InitObject()
	{
		auto box = mmc::mAssetCore.Get<Model>("res/model/outline/box.obj");
		auto floor = mmc::mAssetCore.Get<Model>("res/model/outline/floor.obj");

		auto sprite = new Sprite();
		sprite->SetShader(File::LoadShader("res/shader/outline/normal.shader"));
		sprite->AddMesh(floor->mChilds.at(0)->mMeshs.at(0), floor->mChilds.at(0)->mMaterials.at(0));
		_floorObject = new Object();
		_floorObject->AddComponent(sprite);
		_floorObject->SetParent(&mmc::mRoot);
		_floorObject->GetTransform()->Scale(5, 5, 5);

		auto spriteOutline = new SpriteOutline();
		spriteOutline->SetShader(File::LoadShader("res/shader/outline/normal.shader"));
		spriteOutline->AddMesh(box->mChilds.at(0)->mMeshs.at(0), box->mChilds.at(0)->mMaterials.at(0));
		_boxObject = new Object();
		_boxObject->AddComponent(spriteOutline);
		_boxObject->SetParent(&mmc::mRoot);
		_boxObject->GetTransform()->Translate(0.0f, 0.5f, 0.0f);
	}

	void InitEvents()
	{
		mmc::mTimer.Add(16, std::bind(&AppWindow::OnTimerUpdate, this));
		mmc::mEvent.Add(Window::EventType::kKEYBOARD, std::bind(&AppWindow::OnKeyEvent, this, std::placeholders::_1));
		mmc::mEvent.Add(Window::EventType::kMOUSE_BUTTON, std::bind(&AppWindow::OnMouseButton, this, std::placeholders::_1));
		mmc::mEvent.Add(Window::EventType::kMOUSE_MOVEED, std::bind(&AppWindow::OnMouseMoveed, this, std::placeholders::_1));
	}

	void InitLights()
	{
	}

	void OnKeyEvent(const std::any & any)
	{
		auto & param = std::any_cast<Window::EventKeyParam>(any);
		_direct = param.act == 1 && param.key == 'W' ? _direct | kFRONT :
				  param.act == 0 && param.key == 'W' ? _direct ^ kFRONT : _direct;
		
		_direct = param.act == 1 && param.key == 'S' ? _direct | kBACK:
				  param.act == 0 && param.key == 'S' ? _direct ^ kBACK : _direct;

		_direct = param.act == 1 && param.key == 'A' ? _direct | kLEFT :
				  param.act == 0 && param.key == 'A' ? _direct ^ kLEFT : _direct;

		_direct = param.act == 1 && param.key == 'D' ? _direct | kRIGHT :
				  param.act == 0 && param.key == 'D' ? _direct ^ kRIGHT : _direct;

		_direct = param.act == 1 && param.key == 'Q' ? _direct | kUP :
				  param.act == 0 && param.key == 'Q' ? _direct ^ kUP : _direct;

		_direct = param.act == 1 && param.key == 'E' ? _direct | kDOWN :
				  param.act == 0 && param.key == 'E' ? _direct ^ kDOWN : _direct;
	}

	void OnMouseButton(const std::any & any)
	{
	}

	void OnMouseMoveed(const std::any & any)
	{
		auto  param = std::any_cast<Window::EventMouseParam>(any);
		auto l = glm::vec2(GetW() * 0.5f, GetH() * 0.5f);
		auto v = glm::vec2(param.x - l.x, l.y - param.y);
		if (glm::length(v) < 100)
		{
			_speed = 0.0f;
			_axis.x = 0;
			_axis.y = 0;
			_axis.z = 0;
		}
		else
		{
			auto camera = mmc::mRender.GetCamera(0);
			auto a = glm::dot(glm::vec2(1, 0), glm::normalize(v));
			auto cos = std::acos(glm::dot(glm::vec2(1, 0), glm::normalize(v)));
			cos = v.y < 0 ? cos : -cos;

			auto right = glm::cross(camera->GetEye(), camera->GetUp());
			auto step = glm::angleAxis(cos, camera->GetEye()) * right;
			auto look = glm::normalize(camera->GetEye() + step);
			_axis = glm::normalize(glm::cross(camera->GetEye(), look));

			auto s = glm::length(v) / glm::length(l);
			_speed = s * AppWindow::s_MAX_SPEED;
		}
	}
	
	void OnTimerUpdate()
	{
		auto camera = mmc::mRender.GetCamera(0);
		if (_axis.x != 0 || _axis.y != 0 || _axis.z != 0)
		{
			camera->SetEye(glm::quat(glm::angleAxis(_speed, _axis)) * camera->GetEye());
		}
		if (_direct != 0)
		{
			auto pos = camera->GetPos();
			if ((_direct & kUP) != 0) { pos.y += 1 * 0.1f; }
			if ((_direct & kDOWN) != 0) { pos.y -= 1 * 0.1f; }
			if ((_direct & kFRONT) != 0) { pos += camera->GetEye() * 0.1f; }
			if ((_direct & kBACK) != 0) { pos -= camera->GetEye() * 0.1f; }
			if ((_direct & kLEFT) != 0) { pos += glm::cross(camera->GetUp(), camera->GetEye()) * 0.1f; }
			if ((_direct & kRIGHT) != 0) { pos -= glm::cross(camera->GetUp(), camera->GetEye()) * 0.1f; }
			camera->SetPos(pos);
		}
		mmc::mTimer.Add(16, std::bind(&AppWindow::OnTimerUpdate, this));
	}
	
private:
	Object * _floorObject;
	Object * _boxObject;
	glm::vec3 _axis;
	float _speed;
	int _direct;
};

int main()
{
    AppWindow app;
    app.Create("xxx");
    app.Move(200, 100, 800, 600);
    app.InitGame();
    app.SetFPS(60);
    app.Loop();
    return 0;
}