#pragma once

#include "component.h"

class Transform : public Component {
public:
	Transform();
	~Transform();
    virtual void OnAdd() override;
    virtual void OnDel() override;
    virtual void OnUpdate(float dt) override;

    void Translate(float x, float y, float z);
    void Translate(const glm::vec3 & vec);

    void Rotate(const glm::quat & rotate);
    void Rotate(const glm::vec3 & vec, float a);
    void Rotate(float x, float y, float z, float a);

	void Scale(float x);
    void Scale(float x, float y, float z);
    void Scale(const glm::vec3 & vec);

    Transform & AddTranslate(float x, float y, float z);
    Transform & AddTranslate(const glm::vec3 & vec);

    Transform & AddRotate(const glm::quat & rotate);
    Transform & AddRotate(const glm::vec3 & vec, float a);
    Transform & AddRotate(float x, float y, float z, float a);

    Transform & AddScale(float x, float y, float z);
    Transform & AddScale(const glm::vec3 & vec);
    
    const glm::quat & GetRotateQuat() const;
    const glm::vec3 & GetPosition() const;
    const glm::vec3 & GetScale() const;
    glm::vec3 GetRotate() const;

    const glm::mat4 & GetMatrix();
	glm::mat4 GetMatrixFromRoot();
	glm::mat4 GetRotateFromRoot();
	glm::vec3 GetWorldPosition();
	glm::vec3 ApplyRotate(const glm::vec3 & vec3);

private:
    void UpdateMatrix();

private:
    glm::vec3 _scale;
    glm::quat _rotate;
	glm::vec3 _translate;
	glm::mat4 _matrix;
    bool _isChange;
};