#version 330

// Vertex shader principal (el de iluminacion). Ademas de lo normal,
// deforma los asteroides con ruido para que no se vean lisos

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;

out vec4 vCol;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 vColor;
out vec3 PosLocal; // posicion sin transformar, para la roca procedural de los asteroides

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 color;
// Solo para los asteroides; cada semilla da una roca con otra forma
uniform bool rocaProcedural;
uniform float semilla;

// Numero pseudoaleatorio entre 0 y 1 a partir de un punto
float Hash3(vec3 p)
{
	return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453);
}
// Ruido de valor 3D: valor al azar en las 8 esquinas de la celda, mezclado suave (igual que en el .frag)
float Ruido3(vec3 p)
{
	vec3 i = floor(p), f = fract(p);
	vec3 u = f * f * (3.0 - 2.0 * f);
	return mix(mix(mix(Hash3(i), Hash3(i + vec3(1.0, 0.0, 0.0)), u.x),
	               mix(Hash3(i + vec3(0.0, 1.0, 0.0)), Hash3(i + vec3(1.0, 1.0, 0.0)), u.x), u.y),
	           mix(mix(Hash3(i + vec3(0.0, 0.0, 1.0)), Hash3(i + vec3(1.0, 0.0, 1.0)), u.x),
	               mix(Hash3(i + vec3(0.0, 1.0, 1.0)), Hash3(i + vec3(1.0, 1.0, 1.0)), u.x), u.y), u.z);
}
// Cuanto se mete o se sale la superficie en el punto x: abolladuras grandes, medianas y chicas
float Abolladura(vec3 x)
{
	vec3 p = x * 0.006 + vec3(semilla * 0.731, semilla * 0.417, semilla * 0.913);
	float grande = Ruido3(p);
	float mediana = Ruido3(p * 2.3 + vec3(5.2));
	float chica = Ruido3(p * 5.1 + vec3(9.7));
	return 170.0 * (0.62 * (grande - 0.5) + 0.26 * (mediana - 0.5) + 0.08 * (chica - 0.5));
}


void main()
{
	vec3 posFinal = pos;
	vec3 normalFinal = norm;
	if (rocaProcedural)
	{
		// Cada vertice se empuja hacia afuera o hacia adentro desde el centro del modelo
		vec3 radial = normalize(pos);
		posFinal = pos + radial * Abolladura(pos);
		// Al deformar, la normal original ya no sirve. Se corrige con el gradiente de la
		// abolladura, calculado con diferencias finitas en X, Y y Z
		float e = 4.0;
		float h = Abolladura(pos);
		vec3 gradiente = vec3(Abolladura(pos + vec3(e, 0.0, 0.0)) - h,
		                      Abolladura(pos + vec3(0.0, e, 0.0)) - h,
		                      Abolladura(pos + vec3(0.0, 0.0, e)) - h) / e;
		vec3 n = normalize(norm);
		normalFinal = normalize(n - (gradiente - dot(gradiente, n) * n)); // solo se usa la parte tangente del gradiente
	}
	gl_Position = projection * view * model * vec4(posFinal, 1.0);
	vCol = vec4(0.0, 1.0, 0.0, 1.0f);
	vColor=vec4(color,1.0f);
	TexCoord = tex;
	// Matriz normal: inversa transpuesta del model, para que las normales sigan bien con escalas no uniformes
	Normal = mat3(transpose(inverse(model))) * normalFinal;

	FragPos = (model * vec4(posFinal, 1.0)).xyz; // posicion del fragmento en el mundo (para la luz)
	PosLocal = pos; // sin transformar: asi el patron de la roca se queda pegado al asteroide aunque gire
}
