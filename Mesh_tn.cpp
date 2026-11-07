#include "Mesh_tn.h"

// Tres tipos de malla:
//  Mesh      -> solo posicion, dibujada con indices
//  MeshColor -> posicion + color por vertice, sin indices
//  MeshModel -> posicion + UV + normal (8 floats por vertice), la que usa casi todo el proyecto

Mesh::Mesh()
{
	VAO = 0;
	VBO = 0;
	IBO = 0;
	indexCount = 0;

}
MeshColor::MeshColor()
{
	VAO = 0;
	VBO = 0;
	vertexCount = 0;

}

MeshModel::MeshModel()
{
	VAO = 0;
	VBO = 0;
	IBO = 0;
	indexCount = 0;
}


void Mesh::CreateMesh(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numberOfIndices)
{

	indexCount = numberOfIndices;
	glGenVertexArrays(1, &VAO); // se crea el VAO
	glBindVertexArray(VAO);

	// Indices
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * numberOfIndices, indices, GL_STATIC_DRAW);

	// Vertices; GL_STATIC_DRAW porque no se van a modificar
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * numOfVertices, vertices, GL_STATIC_DRAW);

	// Atributo 0: posicion (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

// Igual que CreateMesh pero recibe vectores en lugar de arreglos
void Mesh::CreateMeshGeometry(std::vector<GLfloat>& vertices, std::vector<unsigned int>& indices, unsigned int numOfVertices, unsigned int numberOfIndices)
{

	indexCount = numberOfIndices;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * numberOfIndices, &indices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * numOfVertices, &vertices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void MeshColor::CreateMeshColor(GLfloat* vertices, unsigned int numOfVertices)
{
	vertexCount = numOfVertices;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * numOfVertices, vertices, GL_STATIC_DRAW);
	// Cada vertice son 6 floats: 3 de posicion y 3 de color
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 6, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 6, (void*)(sizeof(vertices[0]) * 3)); // el color empieza despues de la posicion
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}



void MeshModel::CreateMeshModel(GLfloat* vertices, unsigned int* indices, unsigned int numOfVertices, unsigned int numOfIndices)
{
	indexCount = numOfIndices;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * numOfIndices, indices, GL_STATIC_DRAW);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * numOfVertices, vertices, GL_STATIC_DRAW);
	// Cada vertice: x y z | s t | nx ny nz  (8 floats)
	// Atributo 0: posicion
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, 0);
	glEnableVertexAttribArray(0);
	// Atributo 1: coordenadas de textura (S, T)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, (void*)(sizeof(vertices[0]) * 3));
	glEnableVertexAttribArray(1);
	// Atributo 2: normal del vertice
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]) * 8, (void*)(sizeof(vertices[0]) * 5));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

// Dibuja usando los indices
void Mesh::RenderMesh()
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// Dibuja directo de los vertices (no tiene indices)
void MeshColor::RenderMeshColor()
{
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	glBindVertexArray(0);
}

// Igual que RenderMesh pero con GL_TRIANGLE_FAN
void Mesh::RenderMeshGeometry()
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glDrawElements(GL_TRIANGLE_FAN, indexCount, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



void MeshModel::RenderMeshModel()
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}



// Las funciones Clear liberan los buffers en la GPU para no dejar memoria ocupada
void Mesh::ClearMesh()
{
	if (IBO != 0)
	{
		glDeleteBuffers(1, &IBO);
		IBO = 0;
	}
	if (VBO != 0)
	{
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}
	if (VAO != 0)
	{
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}
	indexCount = 0;
}

void MeshColor::ClearMeshColor()
{

	if (VBO != 0)
	{
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}
	if (VAO != 0)
	{
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}
	vertexCount = 0;
}

void MeshModel::ClearMeshModel()
{
	if (IBO != 0)
	{
		glDeleteBuffers(1, &IBO);
		IBO = 0;
	}

	if (VBO != 0)
	{
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}

	if (VAO != 0)
	{
		glDeleteVertexArrays(1, &VAO);
		VAO = 0;
	}

	indexCount = 0;
}

Mesh::~Mesh()
{
	ClearMesh();
}
MeshColor::~MeshColor()
{
	ClearMeshColor();
}

MeshModel::~MeshModel()
{
	ClearMeshModel();
};
