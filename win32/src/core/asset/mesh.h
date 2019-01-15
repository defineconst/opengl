#pragma once

#include "asset.h"

class Mesh: public Asset {
public:
	struct Vertex {
		glm::vec3 v;
		struct {
			float u;
			float v;
		} uv;
		glm::vec3 n;
	};

public:
	Mesh(std::vector<Vertex> && vertexs)
		: _vertexs(std::move(vertexs))
		, _vao(0), _vbo(0)
	{
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);
		glGenBuffers(1, &_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo);
		glBufferData(GL_ARRAY_BUFFER, _vertexs.size() * sizeof(Vertex), _vertexs.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, v));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, n));
		glEnableVertexAttribArray(2);
		glBindVertexArray(0);
	}

	virtual ~Mesh()
	{
		glDeleteBuffers(1, &_vbo);
		glDeleteVertexArrays(1, &_vao);
		_vbo = 0;
		_vao = 0;
	}

	const std::vector<Vertex> & GetVertexs() const
	{
		return _vertexs;
	}

	GLuint GetGLID() const
	{
		assert(_vao != 0);
		assert(_vbo != 0);
		return _vao;
	}

private:
	GLuint _vao;
	GLuint _vbo;
	std::vector<Vertex> _vertexs;
};