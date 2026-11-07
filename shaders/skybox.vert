#version 330

// Vertex shader del skybox

layout (location = 0) in vec3 pos;
out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view; // llega sin traslacion (ver Skybox::DrawSkybox)
void main()
{
// En un cubemap la coordenada de textura es la direccion desde el centro del cubo.
// Va negada porque asi quedan bien orientadas las caras con las imagenes que usamos
TexCoords= -pos;
gl_Position= projection*view * vec4(pos,1.0);
}
