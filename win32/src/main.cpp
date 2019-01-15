#include "core/mmc.h"
#include "core/timer/timer.h"
#include "core/event/event.h"
#include "core/window/window.h"
#include "core/object/camera.h"
#include "core/render/render.h"
#include "core/asset/shader.h"
#include "core/asset/material.h"
#include "core/component/sprite.h"
#include "core/asset/asset_core.h"
#include "core/tools/debug_tool.h"
#include "core/component/light.h"
#include "core/component/transform.h"
#include "core/asset/file.h"

class AppWindow : public Window {
public:
    void InitGame()
    {
		InitCamera();
		InitAssets();
		InitEvents();
		InitLights();
		InitBoxs();
    }
private:
	void InitCamera()
	{
		auto camera = new Camera();
		camera->Init(90, (float)GetW(), (float)GetH(), 1, 500);
		camera->LookAt(
			glm::vec3(0, 0, 3),
			glm::vec3(0, 0, 0),
			glm::vec3(0, 1, 0));
		mmc::mRender.AddCamera(0, camera);
	}

	void InitAssets()
	{
		File::LoadTexture("res/bitmap/awesomeface.png", GL_RGBA);
		File::LoadTexture("res/bitmap/container.png", GL_RGBA);
		File::LoadMaterial("res/material/1.txt");
		File::LoadShader("res/shader/1.shader");
		File::LoadShader("res/shader/light.shader");
		File::LoadMesh("res/model/1/model.obj");
	}

	void InitBoxs()
	{
		glm::vec3 points[] = {
			glm::vec3(0.0f,  0.0f,  0.0f),
			glm::vec3(2.0f,  5.0f, -15.0f),
			glm::vec3(-1.5f, -2.2f, -2.5f),
			glm::vec3(-3.8f, -2.0f, -12.3f),
			glm::vec3(2.4f, -0.4f, -3.5f),
			glm::vec3(-1.7f,  3.0f, -7.5f),
			glm::vec3(1.3f, -2.0f, -2.5f),
			glm::vec3(1.5f,  2.0f, -2.5f),
			glm::vec3(1.5f,  0.2f, -1.5f),
			glm::vec3(-1.3f,  1.0f, -1.5f)
		};

		for (auto & point : points)
		{
			auto sprite = new Sprite();
			sprite->SetMaterial(mmc::mAssetCore.Get<Material>("res/material/1.txt"));
			sprite->SetShader(mmc::mAssetCore.Get<Shader>("res/shader/1.shader"));
			sprite->SetMesh(mmc::mAssetCore.Get<Mesh>("res/model/1/model.obj"));
			sprite->SetTexture0("res/bitmap/container.png");
			sprite->SetTexture1("res/bitmap/awesomeface.png");

			auto child = new Object();
			child->GetTransform()->Translate(point);
			child->AddComponent(sprite);
			mmc::mRoot.AddChild(child);
		}
	}

	void InitEvents()
	{
		mmc::mEvent.Add(Window::EventType::kKEYBOARD, [this](const std::any & event) {
			auto param = std::any_cast<Window::EventKeyParam>(event);
			if (param.act == 2)
			{
				auto camera = mmc::mRender.GetCamera(0);
				switch (param.key)
				{
				case 'W': { camera->SetPos(camera->GetPos() + camera->GetEye()); }
						  break;
				case 'S': { camera->SetPos(camera->GetPos() - camera->GetEye()); }
						  break;
				case 'A': { camera->SetPos(camera->GetPos() - glm::cross(camera->GetEye(), camera->GetUp())); }
						  break;
				case 'D': { camera->SetPos(camera->GetPos() + glm::cross(camera->GetEye(), camera->GetUp())); }
						  break;
				case 'Q': { camera->SetPos(camera->GetPos() + camera->GetUp()); }
						  break;
				case 'E': { camera->SetPos(camera->GetPos() - camera->GetUp()); }
						  break;
				case 'I': { auto rotate = camera->GetRotate(); rotate.x += glm::radians(5.0f); camera->SetRotate(rotate); }
						  break;
				case 'K': { auto rotate = camera->GetRotate(); rotate.x -= glm::radians(5.0f); camera->SetRotate(rotate); }
						  break;
				case 'J': { auto rotate = camera->GetRotate(); rotate.y -= glm::radians(5.0f); camera->SetRotate(rotate); }
						  break;
				case 'L': { auto rotate = camera->GetRotate(); rotate.y += glm::radians(5.0f); camera->SetRotate(rotate); }
						  break;
				}
			}
		});
	}

	void InitLights()
	{
		auto light = new Light();
		light->mMesh = mmc::mAssetCore.Get<Mesh>("res/model/1/model.obj");
		light->mShader = mmc::mAssetCore.Get<Shader>("res/shader/light.shader");
		light->mAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
		light->mDiffuse = glm::vec3(0.6f, 0.1f, 0.4f);
		light->mSpecular = glm::vec3(1.0f, 1.0f, 1.0f);

		auto lightObject = new Object();
		lightObject->AddComponent(light);
		lightObject->GetTransform()->Scale(0.2f);
		lightObject->GetTransform()->Translate(1.0f, 1.0f, 2.0f);
		lightObject->SetParent(&mmc::mRoot);
	}
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