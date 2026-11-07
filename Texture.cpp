#include "Texture.h"
#include "CommonValues.h"

// Carga imagenes con stb_image y las sube a la GPU como texturas 2D

Texture::Texture()
{
	textureID = 0;
	width = 0;
	height = 0;
	bitDepth = 0;
	fileLocation = 0;
}
Texture::Texture(const char *FileLoc)
{
	textureID = 0;
	width = 0;
	height = 0;
	bitDepth = 0;
	fileLocation = FileLoc;
}

// Version con canal alfa (png y tga); se fuerza a RGBA aunque la imagen no tenga alfa
bool Texture::LoadTextureA()
{
	// stb carga la imagen de arriba hacia abajo y OpenGL espera el origen abajo a la izquierda
	stbi_set_flip_vertically_on_load(true);
	unsigned char *texData = stbi_load(fileLocation, &width, &height, &bitDepth,STBI_rgb_alpha); // bytes de la imagen
	if (!texData)
	{
		printf("No se encontro el archivo: %s", fileLocation);
	}
	glGenTextures(1, &textureID); // parecido al VAO: se crea la textura y se le da un indice
	glBindTexture(GL_TEXTURE_2D, textureID);

	// REPEAT en S y T para que la textura se repita si las UV pasan de 1
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// LINEAR mezcla los texels cercanos para que no se vea pixeleado
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	glGenerateMipmap(GL_TEXTURE_2D); // los planetas los usan para que no parpadeen de lejos
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(texData); // ya esta en la GPU, se libera la memoria
	return true;
}

// Version sin alfa (jpg); se carga en RGB
bool Texture::LoadTexture()
{
	stbi_set_flip_vertically_on_load(true);
	unsigned char *texData = stbi_load(fileLocation, &width, &height, &bitDepth, 0);
	if (!texData)
	{
		printf("No se encontro el archivo: %s", fileLocation);
	}
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(texData);
	return true;
}
void Texture::ClearTexture()
{

	glDeleteTextures(1, &textureID);
	textureID = 0;
	width = 0;
	height = 0;
	bitDepth = 0;
	fileLocation = "";
}

// Activa la textura en la unidad 0, que es la que lee el sampler del shader
void Texture::UseTexture()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

}

Texture::~Texture()
{
	ClearTexture();
}
