
#include "core/timer/timer.h"
#include "core/event/event.h"
#include "core/window/window.h"
#include "core/component/camera.h"
#include "core/render/render.h"
#include "core/res/shader.h"
#include "core/res/material.h"
#include "core/component/sprite.h"
#include "core/component/light.h"
#include "core/component/transform.h"
#include "core/res/model.h"
#include "core/res/file.h"
#include "core/res/res_manager.h"
#include "core/raw/raw_manager.h"
#include "core/cfg/cfg_manager.h"
#include "core/event/event_enum.h"
#include <filesystem>

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
        glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
        glPrimitiveRestartIndex(0xffffffffu);

        InitAssets();
		InitCamera();
		InitEvents();
		InitLights();
		InitObject();
	}
private:
	void InitCamera()
	{
		auto camera = new Camera();
		camera->InitPerspective(60, (float)GetW(), (float)GetH(), 1.0f, 30000);
		camera->SetViewport({ 0, 0, GetW(), GetH() });
		camera->LookAt(
			glm::vec3(0, 10, 10),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0));
        camera->SetMask(Camera::kMASK0);
        camera->SetOrder(0);

        auto object = new Object();
        object->AddComponent(camera);

        Global::Ref().RefObject().AddChild(object);

        _mainCamera = camera;
	}

	void InitAssets()
	{
        Global::Ref().RefRawManager().BegImport(true);
        Global::Ref().RefRawManager().Import("res/demo2/scene.obj");
        Global::Ref().RefRawManager().Import("res/demo2/program/scene.program");
        Global::Ref().RefRawManager().Import("res/demo2/material/scene.mtl");
        Global::Ref().RefRawManager().EndImport();
	}

	void InitObject()
	{
        auto sprite = new Sprite();
        sprite->BindMaterial(Global::Ref().RefRawManager().LoadRes<GLMaterial>("9f8fe62366b62d4083bd4eb1c9af0604"));

        auto object = new Object();
        object->AddComponent(sprite);
        object->SetParent(&Global::Ref().RefObject());
	}

	void InitEvents()
	{
		Global::Ref().RefTimer().Add(0.016f, std::bind(&AppWindow::OnTimerUpdate, this));
		Global::Ref().RefEvent().Add(EventTypeEnum::kWINDOW_KEYBOARD, std::bind(&AppWindow::OnKeyEvent, this, std::placeholders::_1));
		Global::Ref().RefEvent().Add(EventTypeEnum::kWINDOW_MOUSE_BUTTON, std::bind(&AppWindow::OnMouseButton, this, std::placeholders::_1));
		Global::Ref().RefEvent().Add(EventTypeEnum::kWINDOW_MOUSE_MOVEED, std::bind(&AppWindow::OnMouseMoveed, this, std::placeholders::_1));
	}

	void InitLights()
	{
		//	���꣬�����⣬�����䣬���淴�䣬����
		const std::vector<std::array<glm::vec3, 5>> directs = {
			{ glm::vec3(0, 10, 10), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(0.5f, 0.5f, 0.5f), glm::normalize(glm::vec3(0, -1, -1)) },
		};

		//	���꣬�����⣬�����䣬���淴�䣬˥��k0, k1, k2
		const std::vector<std::array<glm::vec3, 5>> points = {
            { glm::vec3(-1.5f, 8, 3), glm::vec3(0.1f, 0.1f, 0.1f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0001f, 0.01f) },
		};

		//	���꣬�����������䣬���淴�䣬����˥��k0, k1, k2�����нǣ����н�
		const std::vector<std::array<glm::vec3, 7>> spots = {
			{ glm::vec3(-1.5f, 10, -3), glm::vec3(0.3f, 0.3f, 0.3f), glm::vec3(0.4f, 0.4f, 0.4f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0, -1, 0), glm::vec3(1.0f, 0.0001f, 0.01f), glm::vec3(0.9f, 0.8f, 0.0f) },
		};

		for (auto & data : directs)
		{
			auto light = new LightDirect();
			light->mAmbient = data[1];
			light->mDiffuse = data[2];
			light->mSpecular = data[3];
			light->mNormal = data[4];
            light->OpenShadow({ -50, 50 }, { -50, 50 }, { -10, 1000 });
			auto object = new Object();
			object->AddComponent(light);
			object->GetTransform()->Translate(data[0]);
			object->SetParent(&Global::Ref().RefObject());
			_lightDirects.push_back(light);
		}

		for (auto & data : points)
		{
			auto light = new LightPoint();
			light->mAmbient = data[1];
			light->mDiffuse = data[2];
			light->mSpecular = data[3];
			light->mK0 = data[4].x;
			light->mK1 = data[4].y;
			light->mK2 = data[4].z;
            light->OpenShadow(1, 100);
			auto object = new Object();
			object->AddComponent(light);
			object->GetTransform()->Translate(data[0]);
			object->SetParent(&Global::Ref().RefObject());
			_lightPoints.push_back(light);
		}

		for (auto & data : spots)
		{
			auto light = new LightSpot();
			light->mAmbient = data[1];
			light->mDiffuse = data[2];
			light->mSpecular = data[3];
			light->mNormal = data[4];
			light->mK0 = data[5].x;
			light->mK1 = data[5].y;
			light->mK2 = data[5].z;
			light->mInCone = data[6].x;
			light->mOutCone = data[6].y;
            light->OpenShadow(1, 100);
			auto object = new Object();
			object->AddComponent(light);
			object->GetTransform()->Translate(data[0]);
			object->SetParent(&Global::Ref().RefObject());
			_lightSpots.push_back(light);
		}
	}

	void OnKeyEvent(const std::any & any)
	{
		auto & param = std::any_cast<const Window::EventKeyParam &>(any);
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
			_axis.x = _axis.y = _axis.z = _speed = 0.0f;
		}
		else
		{
			auto cos = std::acos(glm::dot(glm::vec2(1, 0), glm::normalize(v)));
			cos = v.y < 0 ? cos : -cos;

			auto right = glm::cross(_mainCamera->GetEye(), _mainCamera->GetUp());
			auto step = glm::angleAxis(cos, _mainCamera->GetEye()) * right;
			auto look = glm::normalize(_mainCamera->GetEye() + step);
			_axis = glm::normalize(glm::cross(_mainCamera->GetEye(), look));

			auto s = glm::length(v) / glm::length(l);
			_speed = s * AppWindow::s_MAX_SPEED;
		}
	}
	
	void OnTimerUpdate()
	{
		if (_axis.x != 0 || _axis.y != 0 || _axis.z != 0)
		{
            _mainCamera->SetEye(glm::quat(glm::angleAxis(_speed, _axis)) * _mainCamera->GetEye());
		}
		if (_direct != 0)
		{
			auto pos = _mainCamera->GetPos();
			if ((_direct & kUP) != 0) { pos.y += 1 * 0.1f; }
			if ((_direct & kDOWN) != 0) { pos.y -= 1 * 0.1f; }
			if ((_direct & kFRONT) != 0) { pos += _mainCamera->GetEye() * 0.1f; }
			if ((_direct & kBACK) != 0) { pos -= _mainCamera->GetEye() * 0.1f; }
			if ((_direct & kLEFT) != 0) { pos += glm::cross(_mainCamera->GetUp(), _mainCamera->GetEye()) * 0.1f; }
			if ((_direct & kRIGHT) != 0) { pos -= glm::cross(_mainCamera->GetUp(), _mainCamera->GetEye()) * 0.1f; }
            _mainCamera->SetPos(pos);
		}

		Global::Ref().RefTimer().Add(0.016f, std::bind(&AppWindow::OnTimerUpdate, this));
	}

private:
    Camera * _mainCamera;

    //  Lights
	std::vector<LightDirect *> _lightDirects;
	std::vector<LightPoint *> _lightPoints;
	std::vector<LightSpot *> _lightSpots;

    //  Key Input
	glm::vec3 _axis;
	float _speed;
	int _direct;
};

int main()
{
    Global::Ref().Start();

    auto renderFPS = Global::Ref().RefCfgManager().At("init")->At("render", "fps")->ToInt();

    auto windowX = Global::Ref().RefCfgManager().At("init")->At("window", "x")->ToInt();
    auto windowY = Global::Ref().RefCfgManager().At("init")->At("window", "y")->ToInt();
    auto windowW = Global::Ref().RefCfgManager().At("init")->At("window", "w")->ToInt();
    auto windowH = Global::Ref().RefCfgManager().At("init")->At("window", "h")->ToInt();
    auto windowTitle = Global::Ref().RefCfgManager().At("init")->At("window", "title")->ToString();

    AppWindow app;
    app.Create(windowTitle);
    app.Move(windowX, windowY, 
             windowW, windowH);
    app.SetFPS(renderFPS);
    app.InitGame();
    app.Loop();

    Global::Ref().Clean();

    return 0;
}