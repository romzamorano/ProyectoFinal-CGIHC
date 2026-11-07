#version 330
in vec4 vColor;
in vec2 TexCoord;

out vec4 color;
uniform sampler2D theTexture;

void main()
{

	// Textura multiplicada por el color, asi se puede cambiar el tono del objeto desde C++
	color = vColor*texture(theTexture, TexCoord);

}
