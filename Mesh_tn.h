#pragma once
#include <glew.h>
#include <vector>

// Malla solo con posicion, dibujada con indices
class Mesh
{
public:
	Mesh();

	void CreateMesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numberOfIndices);
	void CreateMeshGeometry(std::vector<GLfloat>& vertices, std::vector<unsigned int>& indices, unsigned int numOfVertices, unsigned int numberOfIndices);
	void RenderMesh();
	void RenderMeshGeometry();
	void ClearMesh();
	~Mesh();
private:
	GLuint VAO, VBO, IBO;
	GLsizei indexCount;

};

// Malla con color por vertice, sin indices
class MeshColor
{
public:
	MeshColor();
	void CreateMeshColor(GLfloat* vertices, unsigned int numOfVertices);
	void RenderMeshColor();
	void ClearMeshColor();
	~MeshColor();
private:
	GLuint VAO, VBO;
	GLsizei  vertexCount;
};

// Malla con posicion, UV y normal; la usan los modelos, los planetas y el skybox
class MeshModel
{
public:
	MeshModel();

	void CreateMeshModel(GLfloat *vertices, unsigned int *indices, unsigned int numOfVertices, unsigned int numOfIndices);
	void RenderMeshModel();
	void ClearMeshModel();

	~MeshModel();

private:
	GLuint VAO, VBO, IBO;
	GLsizei indexCount;
};

