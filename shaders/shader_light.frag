#version 330

// Fragment shader principal: iluminacion de Phong (direccional + puntuales + spotlights),
// mas lo que se le agrego para el proyecto: emision, transparencia, textura triplanar
// para los planetas y roca procedural para los asteroides

in vec4 vCol;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 vColor;
in vec3 PosLocal; // posicion en coordenadas del modelo

out vec4 color;

// Deben ser iguales a los de CommonValues.h
const int MAX_POINT_LIGHTS = 7;
const int MAX_SPOT_LIGHTS = 8; // linterna del avatar + 6 reflectores de la pista

// Mismos structs que las clases de C++ (Light, DirectionalLight, PointLight, SpotLight)
struct Light
{
	vec3 color;
	float ambientIntensity;
	float diffuseIntensity;
};

struct DirectionalLight
{
	Light base;
	vec3 direction;
};

struct PointLight
{
	Light base;
	vec3 position;
	float constant;
	float linear;
	float exponent;
};


struct SpotLight
{
	PointLight base;
	vec3 direction;
	float edge; // coseno del angulo del cono
};

struct Material
{
	float specularIntensity;
	float shininess;
};

uniform int pointLightCount;
uniform int spotLightCount;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

uniform sampler2D theTexture;
uniform Material material;

uniform vec3 eyePosition;
// Emision: brillo propio que no depende de las luces (llamas, balizas, focos)
uniform vec3 emision = vec3(0.0);
uniform float opacidad = 1.0; // menor a 1 para el humo
// Mapa de emision en la unidad de textura 1 (ventanas de la estacion, cristales de la mina)
uniform sampler2D mapaEmision;
uniform bool usarMapaEmision = false;
// Textura de los planetas proyectada en los tres planos, para que se vea igual en el suelo y en la esfera
uniform bool texturaTerreno;
uniform float corteAlfa; // los pixeles con alfa menor a esto se descartan (hojas de las plantas)
uniform vec3 origenTerreno;

// Ruido 2D suave: varia el tono del terreno por zonas grandes
float Ruido(vec2 p)
{
    vec2 i = floor(p), f = fract(p);
    float a = fract(sin(dot(i, vec2(127.1, 311.7))) * 43758.5453);
    float b = fract(sin(dot(i + vec2(1.0, 0.0), vec2(127.1, 311.7))) * 43758.5453);
    float c = fract(sin(dot(i + vec2(0.0, 1.0), vec2(127.1, 311.7))) * 43758.5453);
    float d = fract(sin(dot(i + vec2(1.0, 1.0), vec2(127.1, 311.7))) * 43758.5453);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

// ---------- Roca procedural de los asteroides ----------
// El color se calcula con funciones de ruido, sin imagen. Cada asteroide manda
// una semilla distinta, asi la misma funcion da otro patron y otro tono
uniform bool rocaProcedural;
uniform float semilla;
// Un numero pseudoaleatorio entre 0 y 1 a partir de un punto 3D
float Hash3(vec3 p)
{
    return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453);
}
// Tres numeros pseudoaleatorios a partir de un punto 3D
vec3 Hash33(vec3 p)
{
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)), dot(p, vec3(269.5, 183.3, 246.1)), dot(p, vec3(113.5, 271.9, 124.6)));
    return fract(sin(p) * 43758.5453);
}
// Ruido de valor 3D: valor al azar en las 8 esquinas de la celda, mezclado suave
float Ruido3(vec3 p)
{
    vec3 i = floor(p), f = fract(p);
    vec3 u = f * f * (3.0 - 2.0 * f);
    float n000 = Hash3(i);
    float n100 = Hash3(i + vec3(1.0, 0.0, 0.0));
    float n010 = Hash3(i + vec3(0.0, 1.0, 0.0));
    float n110 = Hash3(i + vec3(1.0, 1.0, 0.0));
    float n001 = Hash3(i + vec3(0.0, 0.0, 1.0));
    float n101 = Hash3(i + vec3(1.0, 0.0, 1.0));
    float n011 = Hash3(i + vec3(0.0, 1.0, 1.0));
    float n111 = Hash3(i + vec3(1.0, 1.0, 1.0));
    return mix(mix(mix(n000, n100, u.x), mix(n010, n110, u.x), u.y),
               mix(mix(n001, n101, u.x), mix(n011, n111, u.x), u.y), u.z);
}
// fBm: suma 5 capas de ruido, cada una con el doble de detalle y la mitad de fuerza
float Fbm3(vec3 p)
{
    float suma = 0.0;
    float amplitud = 0.5;
    for (int k = 0; k < 5; k++)
    {
        suma += amplitud * Ruido3(p);
        p *= 2.03;
        amplitud *= 0.5;
    }
    return suma;
}
// Crateres con ruido de Worley: busca el punto al azar mas cercano entre las 27 celdas vecinas.
// Regresa x = que tan dentro del hoyo esta (0 a 1) y y = brillo del borde
vec2 Crateres(vec3 p)
{
    vec3 i = floor(p), f = fract(p);
    float masCerca = 8.0;
    float radio = 0.5;
    for (int z = -1; z <= 1; z++)
        for (int y = -1; y <= 1; y++)
            for (int x = -1; x <= 1; x++)
            {
                vec3 celda = vec3(float(x), float(y), float(z));
                vec3 centro = celda + Hash33(i + celda) - f;
                float d = length(centro);
                if (d < masCerca)
                {
                    masCerca = d;
                    radio = 0.25 + 0.35 * Hash3(i + celda + vec3(7.1)); // cada crater de otro tamano
                }
            }
    float t = masCerca / radio;
    float hoyo = 1.0 - smoothstep(0.0, 1.0, t);
    float borde = exp(-pow((t - 1.0) / 0.25, 2.0)); // campana alrededor de t = 1 (la orilla)
    return vec2(hoyo, borde);
}
vec4 RocaProcedural()
{
    vec3 p = PosLocal * 0.012 + vec3(semilla * 1.731, semilla * 0.917, semilla * 1.313); // la semilla mueve el patron
    float base = Fbm3(p);                       // manchas grandes y medianas
    float grano = Ruido3(p * 9.0);              // textura fina de la piedra
    vec2 grandes = Crateres(p * 1.6);
    vec2 chicos = Crateres(p * 4.0 + vec3(11.0));
    float v = 0.35 + 0.75 * (base - 0.5) + 0.12 * (grano - 0.5);
    v -= 0.35 * grandes.x - 0.25 * grandes.y;   // el hoyo se oscurece y el borde se aclara
    v -= 0.20 * chicos.x - 0.12 * chicos.y;
    v = clamp(v, 0.0, 1.0);
    // Tono de cada asteroide, de gris azulado a cafe rojizo
    float tono = Hash3(vec3(semilla, 3.7, 1.9));
    vec3 oscuro = mix(vec3(0.17, 0.17, 0.19), vec3(0.22, 0.15, 0.11), tono);
    vec3 claro = mix(vec3(0.62, 0.62, 0.64), vec3(0.70, 0.56, 0.44), tono);
    return vec4(mix(oscuro, claro, v), 1.0);
}

// ---------- Textura de los planetas ----------
// Proyeccion triplanar: la textura se proyecta desde X, Y y Z y se mezcla segun hacia donde apunta la normal
vec4 Triplanar(vec3 p, vec3 pesos)
{
    return texture(theTexture, p.zy) * pesos.x +
        texture(theTexture, p.xz) * pesos.y +
        texture(theTexture, p.xy) * pesos.z;
}
// Decide de donde sale el color del objeto: roca procedural, terreno triplanar o su textura normal
vec4 TexturaSuperficie()
{
    if (rocaProcedural) return RocaProcedural();
    if (!texturaTerreno) return texture(theTexture, TexCoord);
    vec3 local = FragPos - origenTerreno;
    // Pesos de cada plano; la potencia 4 hace que la transicion entre planos sea mas corta
    vec3 pesos = pow(abs(normalize(Normal)), vec3(4.0));
    pesos /= max(pesos.x + pesos.y + pesos.z, 0.0001);
    // Dos capas de la misma textura a distinta escala para que no se note que se repite
    vec4 cerca = Triplanar(local * 0.02, pesos);                            // se repite cada ~50 unidades
    vec4 lejos = Triplanar(local * 0.0045 + vec3(0.37, 0.21, 0.53), pesos); // 4.5 veces mas grande y desfasada
    float distancia = length(eyePosition - FragPos);
    float mezcla = 0.35 + 0.5 * clamp((distancia - 60.0) / 400.0, 0.0, 1.0); // de lejos domina la capa grande
    vec4 color = mix(cerca, lejos, mezcla);
    float tono = mix(0.8, 1.15, Ruido(local.xz * 0.006));                  // zonas mas claras y mas oscuras
    return vec4(color.rgb * tono, color.a);
}

// ---------- Iluminacion ----------
// Ambiental + difusa + especular para una luz que llega desde 'direction'
vec4 CalcLightByDirection(Light light, vec3 direction)
{
	vec4 ambientcolor = vec4(light.color, 1.0f) * light.ambientIntensity;
	// Difusa: coseno del angulo entre la normal y la luz (producto punto de vectores normalizados)
	float diffuseFactor = max(dot(normalize(Normal), normalize(direction)), 0.0f);
	vec4 diffusecolor = vec4(light.color * light.diffuseIntensity * diffuseFactor, 1.0f);

	vec4 specularcolor = vec4(0, 0, 0, 0);

	// Solo hay brillo especular si la cara recibe luz; depende de donde esta la camara
	if(diffuseFactor > 0.0f)
	{
		vec3 fragToEye = normalize(eyePosition - FragPos);
		vec3 reflectedVertex = normalize(reflect(direction, normalize(Normal)));

		float specularFactor = dot(fragToEye, reflectedVertex);
		if(specularFactor > 0.0f)
		{
			specularFactor = pow(specularFactor, material.shininess);
			specularcolor = vec4(light.color * material.specularIntensity * specularFactor, 1.0f);
		}
	}

	return (ambientcolor + diffusecolor + specularcolor);
}

vec4 CalcDirectionalLight()
{
	return CalcLightByDirection(directionalLight.base, directionalLight.direction);
}

// Luz puntual: igual que la direccional pero se divide entre la atenuacion por distancia
vec4 CalcPointLight(PointLight pLight)
{
	vec3 direction = FragPos - pLight.position;
	float distance = length(direction);
	direction = normalize(direction);

	vec4 color = CalcLightByDirection(pLight.base, direction);
	float attenuation = pLight.exponent * distance * distance +
						pLight.linear * distance +
						pLight.constant;

	return (color / attenuation);
}

// Spotlight: solo ilumina dentro del cono, y se desvanece hacia la orilla
vec4 CalcSpotLight(SpotLight sLight)
{
	vec3 rayDirection = normalize(FragPos - sLight.base.position);
	float slFactor = dot(rayDirection, sLight.direction);

	if(slFactor > sLight.edge)
	{
		vec4 color = CalcPointLight(sLight.base);

		// Reescala slFactor de [edge, 1] a [0, 1] para que la orilla del cono no se vea cortada
		return color * (1.0f - (1.0f - slFactor)*(1.0f/(1.0f - sLight.edge)));

	} else {
		return vec4(0, 0, 0, 0);
	}
}

vec4 CalcPointLights()
{
	vec4 totalcolor = vec4(0, 0, 0, 0);
	for(int i = 0; i < pointLightCount; i++)
	{
		totalcolor += CalcPointLight(pointLights[i]);
	}

	return totalcolor;
}

vec4 CalcSpotLights()
{
	vec4 totalcolor = vec4(0, 0, 0, 0);
	for(int i = 0; i < spotLightCount; i++)
	{
		totalcolor += CalcSpotLight(spotLights[i]);
	}

	return totalcolor;
}


void main()
{
	vec4 finalcolor = CalcDirectionalLight();
	finalcolor += CalcPointLights();
	finalcolor += CalcSpotLights();
	vec4 superficie = TexturaSuperficie();
    if (superficie.a < corteAlfa) discard;
    // El mapa de emision se suma aparte, asi brilla aunque no le llegue luz
    vec3 luzPropia = usarMapaEmision ? texture(mapaEmision, TexCoord).rgb * 1.5 : vec3(0.0);
    // Color final = brillo propio + textura * color del objeto * (luces + emision)
    color = vec4(luzPropia + superficie.rgb * vColor.rgb * (finalcolor.rgb + emision),
        superficie.a * opacidad);

}
