///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef TEXTURED_GEOMETRY

struct Light
{
	unsigned int	type;
	vec3			color;
	vec3			direction;
	vec3			position;
};

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
//layout(location=3) in vec3 aTangent;
//layout(location=4) in vec3 aBitangent;

layout(binding = 0, std140) uniform GlobalParams
{
	vec3			uCameraPosition;
	unsigned int	uLightCount;
	Light			uLight[16];
};

layout(binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorlViewProjectionMatrix;
};

uniform mat4 uObjMatrix;
//uniform mat4 uWorldMatrix;
//uniform mat4 uViewProjectMatrix;

out vec2 vTexCoord;
out vec3 vPosition;
out vec3 vNormal;
out vec3 mNormal;
out vec3 vViewDir;

float AMBIENT_FACTOR = 0.2;
float SPECULAR_FACTOR = 0.1;
float DIFFUSE_FACTOR = 0.7;

void main()
{

	vTexCoord	= aTexCoord;
	vPosition	= vec3(uWorldMatrix * vec4(aPosition, 1.0));
	vNormal		= mat3(transpose(inverse(uWorldMatrix))) * aNormal;
	//vNormal		= vec3(uWorldMatrix * vec4(aNormal, 1.0));
	mNormal = vNormal;
	vViewDir	= normalize(uCameraPosition - vPosition);
	gl_Position = uWorlViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////
layout (location = 0) out vec4 gDifusse;		
layout (location = 1) out vec4 gNormal;		

in vec2 vTexCoord;
in vec3 vPosition;
in vec3 vNormal;
in vec3 vViewDir;
in vec3 mNormal;

uniform sampler2D uTexture;

layout(binding = 0, std140) uniform GlobalParams
{
	vec3			uCameraPosition;
	unsigned int	uLightCount;
	Light			uLight[16];
};

//out vec4 oColor;

vec3 CalculateDirLight(Light light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

	// Combine
	vec3 ambient = 0.1 * texture(uTexture, vTexCoord).rgb;
	vec3 diffuse = diff * vec3(texture(uTexture, vTexCoord));
	vec3 specular = 0.5 * spec * light.color;

	return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
	float constant = 1.0;
	float linear = 0.09;
	float quadratic = 0.032;

    // Diffuse Shading
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular Shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    // Attenuation
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
				 
    // Combine Results
    vec3 ambient  = 0.1  * texture(uTexture, vTexCoord).rgb;
    vec3 diffuse  = diff * vec3(texture(uTexture, vTexCoord));
    vec3 specular = 0.5 * spec * light.color;
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

void main()
{
	// Properties
	vec3 norm = normalize(vNormal);
	vec3 viewDir = normalize(vViewDir - vPosition);

	// Lights
	vec3 result = vec3(0,0,0);
	for(int i = 0; i < uLightCount; i++)
	{
		if(uLight[i].type == 0)
			result += CalculateDirLight(uLight[i], norm, viewDir);
		else
			result += CalculatePointLight(uLight[i], norm, vPosition, viewDir);
	}

	gDifusse = vec4(result, 1.0);
	gNormal = vec4(mNormal,1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
