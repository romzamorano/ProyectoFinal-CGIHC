#version 330
// Pinta el fragmento con el color que llega del vertex shader
in vec4 vColor;
out vec4 color;
void main()
{
	color = vColor;
}
