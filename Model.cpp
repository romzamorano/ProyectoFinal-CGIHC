#include "Model.h"
#include <unordered_map>

// Carga modelos (.obj) con Assimp: cada malla del archivo se vuelve un MeshModel
// y cada material se vuelve una textura


Model::Model()
{
}

void Model::LoadModel(const std::string& fileName)
{
	// Triangulate: convierte poligonos y quads a triangulos
	// GenSmoothNormals: genera normales si el modelo no las trae
	// JoinIdenticalVertices: junta los vertices repetidos en uno solo
	// No se usa FlipUVs porque con nuestras texturas ya salen bien
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
	if (!scene)
	{
		printf("Fallo al cargar %s: %s\n", fileName.c_str(), importer.GetErrorString());
		return;
	}
	LoadNode(scene->mRootNode, scene);
	LoadMaterials(scene);
}

// Libera las mallas y texturas del modelo
void Model::ClearModel()
{
	for (unsigned int i = 0; i < MeshList.size(); i++)
	{
		if (MeshList[i])
		{
			delete MeshList[i];
			MeshList[i] = nullptr;

		}
	}

	for (unsigned int i = 0; i < TextureList.size(); i++)
	{
		if (TextureList[i])
		{
			delete TextureList[i];
			TextureList[i] = nullptr;
		}
	}

}

// Dibuja cada malla con la textura de su material
void Model::RenderModel()
{
	for (unsigned int i = 0; i < MeshList.size(); i++)
	{
		unsigned int materialIndex = meshTotex[i];
		if (materialIndex < TextureList.size() && TextureList[materialIndex])
		{
			TextureList[materialIndex]->UseTexture();
		}
		MeshList[i]->RenderMeshModel();

	}


}


Model::~Model()
{
}

// Recorre el arbol de nodos del archivo de forma recursiva
void Model::LoadNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		LoadNode(node->mChildren[i], scene);
	}
}

void Model::LoadMesh(aiMesh* mesh, const aiScene* scene)
{

	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		// Posicion
		vertices.insert(vertices.end(), { mesh->mVertices[i].x,mesh->mVertices[i].y ,mesh->mVertices[i].z });
		// Se guarda la posicion y se actualiza la caja envolvente (minimo y maximo),
		// que se usa para saber cuanto subir cada modelo para que pise el suelo
		glm::vec3 posicion(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		posiciones.push_back(posicion);
		minimo = glm::min(minimo, posicion);
		maximo = glm::max(maximo, posicion);
		// UV; si el modelo no tiene, se pone (0, 0)
		if (mesh->mTextureCoords[0])
		{
			vertices.insert(vertices.end(), { mesh->mTextureCoords[0][i].x,mesh->mTextureCoords[0][i].y });
		}
		else
		{
			vertices.insert(vertices.end(), { 0.0f,0.0f });
		}
		// Normales invertidas: el shader de la clase las espera asi
		vertices.insert(vertices.end(), { -mesh->mNormals[i].x,-mesh->mNormals[i].y ,-mesh->mNormals[i].z });
	}
	// Indices de cada cara (ya son triangulos por aiProcess_Triangulate)
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	MeshModel* newMesh = new MeshModel();
	newMesh->CreateMeshModel(&vertices[0], &indices[0], vertices.size(), indices.size());
	MeshList.push_back(newMesh);
	meshTotex.push_back(mesh->mMaterialIndex); // para saber que textura usa esta malla
}

void Model::LoadMaterials(const aiScene* scene)
{
	// Guarda las texturas ya cargadas para no cargar la misma imagen dos veces
	std::unordered_map<std::string, Texture*> loadedTextures;
	TextureList.resize(scene->mNumMaterials);
	for (unsigned int i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* material = scene->mMaterials[i];
		TextureList[i] = nullptr;
		if (material->GetTextureCount(aiTextureType_DIFFUSE))
		{
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				// Solo se usa el nombre del archivo; la ruta del .mtl se ignora y se busca en Textures/
				std::string filename(path.C_Str());
				const std::size_t separador = filename.find_last_of("/\\");
				if (separador != std::string::npos) filename = filename.substr(separador + 1);

				std::string tga = "tga";
				std::string png = "png";
				std::size_t existetga = filename.find(tga);
				std::size_t existepng = filename.find(png);
				std::string texPath = std::string("Textures/") + filename;

				auto it = loadedTextures.find(texPath);
				if (it != loadedTextures.end())
				{
					TextureList[i] = it->second; // ya estaba cargada, se reutiliza
				}
				else
				{
					Texture* newTex = new Texture(texPath.c_str());
					std::string ext = filename.substr(filename.find_last_of('.') + 1);

					// tga y png pueden tener canal alfa, se cargan con LoadTextureA
					bool loaded = false;
					if (ext == "tga" || ext == "png")
					{
						loaded = newTex->LoadTextureA();
					}
					else
					{
						loaded = newTex->LoadTexture();
					}

					if (loaded)
					{
						TextureList[i] = newTex;
						loadedTextures[texPath] = newTex;
					}
					else
					{
						printf("Fallo en cargar la Textura :%s\n", texPath.c_str());
						delete newTex;
					}
				}
			}
		}
		// Si el material no tiene textura o no se pudo cargar, se usa plain.png
		if (!TextureList[i])
		{
			TextureList[i] = new Texture("Textures/plain.png");
			TextureList[i]->LoadTextureA();
		}

	}
}
