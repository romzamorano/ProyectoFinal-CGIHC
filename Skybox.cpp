#include "Skybox.h"

// Skybox: un cubo con una textura cubemap (6 imagenes) que siempre rodea a la camara



Skybox::Skybox()
{
}

// Recibe las 6 caras en el orden de GL_TEXTURE_CUBE_MAP: +X, -X, +Y, -Y, +Z, -Z
Skybox::Skybox(std::vector<std::string> faceLocations)
{
	// El skybox tiene su propio shader, aparte del de iluminacion
	skyShader = new Shader();
	skyShader->CreateFromFiles("shaders/skybox.vert", "shaders/skybox.frag");
	uniformProjection = skyShader->GetProjectionLocation();
	uniformView = skyShader->GetViewLocation();

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
	int width, height, bitDepth;
	for (size_t i = 0; i < 6; i++)
	{

		unsigned char *texData = stbi_load(faceLocations[i].c_str(), &width, &height, &bitDepth, 0); // bytes de la imagen
		if (!texData)
		{
			printf("No se encontro : %s", faceLocations[i].c_str());
			return;
		}
		// Si la imagen trae canal alfa se carga como RGBA, si no como RGB
		GLenum formato = (bitDepth == 4) ? GL_RGBA : GL_RGB;
		// Las caras del cubemap son constantes seguidas, por eso basta con sumar i
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, formato, width, height, 0, formato, GL_UNSIGNED_BYTE, texData);
		stbi_image_free(texData); // ya se paso a la GPU, se libera
	}
	// CLAMP_TO_EDGE evita que se vean lineas en las uniones entre caras
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Cubo de lado 2 centrado en el origen
	unsigned int skyboxIndices[] = {
		// front
		0, 1, 2,
		2, 1, 3,
		// right
		2, 3, 5,
		5, 3, 7,
		// back
		5, 7, 4,
		4, 7, 6,
		// left
		4, 6, 0,
		0, 6, 1,
		// top
		4, 0, 5,
		5, 0, 2,
		// bottom
		1, 6, 3,
		3, 6, 7
	};

	// Se usa el formato de MeshModel (8 floats), pero UV y normales van en 0:
	// el shader del skybox toma la coordenada del cubemap de la posicion
	float skyboxVertices[] = {
		-1.0f, 1.0f, -1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//v0
		-1.0f, -1.0f, -1.0f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V1
		1.0f, 1.0f, -1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V2
		1.0f, -1.0f, -1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V3
		-1.0f, 1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V4
		1.0f, 1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V5
		-1.0f, -1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V6
		1.0f, -1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f//V7
	};
	skyMesh = new MeshModel();
	skyMesh->CreateMeshModel(skyboxVertices, skyboxIndices, 64, 36);

}

void Skybox::DrawSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
	// Se quita la traslacion de la vista (solo queda la rotacion) para que
	// el cielo se mueva al girar la camara pero nunca se pueda alcanzar
	viewMatrix = glm::mat4(glm::mat3(viewMatrix));

	// Sin escribir en el depth buffer, asi todo lo demas se dibuja encima
	glDepthMask(false);
	skyShader->UseShader();
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
	skyMesh->RenderMeshModel();
	glDepthMask(true);

}


Skybox::~Skybox()
{
}
