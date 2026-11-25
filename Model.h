#pragma once
#include <vector>
#include<string>
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>
#include <glm.hpp>

#include "Mesh_tn.h"
#include "Texture.h"

// Modelo cargado de archivo con Assimp (varias mallas y sus texturas)
class Model
{
public:
	Model();

	void LoadModel(const std::string& fileName);
	void RenderModel();
	bool EstaCargado() const { return !MeshList.empty(); } // si no cargo ninguna malla, fallo la carga
	void ClearModel();
	// Caja envolvente en unidades del archivo; sirve para apoyar los modelos en el piso
	glm::vec3 getMinimo() const { return minimo; }
	glm::vec3 getMaximo() const { return maximo; }
	const std::vector<glm::vec3>& getVertices() const { return posiciones; }

	~Model();

private:
	void LoadNode(aiNode* node, const aiScene* scene);
	void LoadMesh(aiMesh* mesh, const aiScene* scene);
	void LoadMaterials(const aiScene* scene);
	std::vector<MeshModel*>MeshList;
	std::vector<Texture*>TextureList;
	std::vector<unsigned int>meshTotex; // que textura le toca a cada malla
	glm::vec3 minimo{ 1e30f }, maximo{ -1e30f }; // empiezan al reves para que el primer vertice los reemplace
	std::vector<glm::vec3> posiciones;
};
