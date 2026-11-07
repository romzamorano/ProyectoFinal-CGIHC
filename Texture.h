#pragma once
#include<glew.h>

// Textura 2D cargada desde un archivo de imagen
class Texture
{
public:
	Texture();
	Texture(const char* FileLoc);
	bool LoadTexture();   // sin canal alfa (jpg)
	bool LoadTextureA();  // con canal alfa (png, tga)
	void UseTexture();
	void ClearTexture();
	~Texture();
private:
	GLuint textureID;
	int width, height, bitDepth;
	const char *fileLocation;

};

