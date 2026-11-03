#pragma once
#include <vector>
#include<string>
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>
#include <glm.hpp> // para la caja envolvente y las posiciones de los vertices

#include "Mesh_tn.h"
#include "Texture.h"

class Model
{
public:
	Model();

	void LoadModel(const std::string& fileName);
	void RenderModel();
	bool EstaCargado() const { return !MeshList.empty(); }
	void ClearModel();
	glm::vec3 getMinimo() const { return minimo; } // esquina minima de la caja envolvente, en unidades del archivo
	glm::vec3 getMaximo() const { return maximo; } // esquina maxima de la caja envolvente, en unidades del archivo
	const std::vector<glm::vec3>& getVertices() const { return posiciones; } // posiciones de todos los vertices cargados

	~Model();

private:
	void LoadNode(aiNode* node, const aiScene* scene); //assimp
	void LoadMesh(aiMesh* mesh, const aiScene* scene);
	void LoadMaterials(const aiScene* scene);
	std::vector<MeshModel*>MeshList;
	std::vector<Texture*>TextureList;
	std::vector<unsigned int>meshTotex;
	glm::vec3 minimo{ 1e30f }, maximo{ -1e30f }; // caja envolvente del modelo
	std::vector<glm::vec3> posiciones; // vertices guardados para medir el modelo despues de cargarlo
};