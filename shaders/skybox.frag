#version 330
// Toma el color del cubemap en la direccion que llega del vertex shader
in vec3 TexCoords;
out vec4 color;
uniform samplerCube skybox;
void main()
{
color=texture(skybox,TexCoords);
}
