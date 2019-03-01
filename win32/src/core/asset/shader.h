#pragma once

#include "asset.h"

class Texture;
class Bitmap;
class BitmapCube;

class Shader: public Asset {
public:
    Shader(const std::string & vs, const std::string & fs, const std::string & gs = std::string());
    ~Shader();

    void SetUniform(size_t idx, int val);
    void SetUniform(size_t idx, float val);
    void SetUniform(size_t idx, const glm::vec3 & val);
    void SetUniform(size_t idx, const glm::vec4 & val);
	void SetUniform(size_t idx, const glm::mat3 & val);
	void SetUniform(size_t idx, const glm::mat4 & val);
	void SetUniform(size_t idx, const Bitmap * val, size_t pos);
	void SetUniform(size_t idx, const Texture & val, size_t pos);
	void SetUniform(size_t idx, const BitmapCube * val, size_t pos);

    void SetUniform(const std::string & key, int val);
    void SetUniform(const std::string & key, float val);
    void SetUniform(const std::string & key, const glm::vec3 & val);
	void SetUniform(const std::string & key, const glm::vec4 & val);
	void SetUniform(const std::string & key, const glm::mat3 & val);
	void SetUniform(const std::string & key, const glm::mat4 & val);
	void SetUniform(const std::string & key, const Bitmap * val, size_t pos);
	void SetUniform(const std::string & key, const Texture & val, size_t pos);
	void SetUniform(const std::string & key, const BitmapCube * val, size_t pos);

    GLuint GetGLID() const 
	{ 
		assert(_GLID != 0);
		return _GLID; 
	}

private:
	void Init(const char * vs, const char * fs, const char * gs);
	void Check(GLuint shader, const std::string & string);

private:
    GLuint _GLID;
};