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

out vec2 vTexCoord;

void main()
{
	
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////
out vec4 oColor;

in vec2 vTexCoord;
uniform sampler2D gDiffuse;
uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform sampler2D gPosition;

uniform samplerCube environmentMap; 
uniform samplerCube irradianceMap;
uniform mat4 viewMatrix;
uniform int renderMode;

uniform float roughness= 1.0;
uniform vec3 F0 = vec3(0.5);
vec3 vPosition;
vec3 V; // Camera to position
const float PI = 3.14159265359;

layout(binding = 1, std140) uniform GlobalParams
{
	vec3			uCameraPosition;
	unsigned int	uLightCount;
	Light			uLight[16];
};

float LinearizeDepth(vec2 uv)
{
  float n = 1.0; // camera z near
  float f = 100.0; // camera z far
  float z = texture2D(gDepth, uv).x;
  return (2.0 * n) / (f + n - z * (f - n));	
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
} 

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}  

vec3 CalculateDirLight(Light light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

	// Combine
	vec3 ambient = 0.1 * texture(irradianceMap, reflect(-V, normal)).xyz * light.color;
	vec3 diffuse = diff * vec3(texture(gDiffuse, vTexCoord)) * light.color;
	vec3 specular = 0.5 * spec * light.color;

vec3 vPosition = texture(gPosition, vTexCoord).xyz;
	vec3 V = normalize(uCameraPosition - vPosition);

	float cosTheta = dot(normal, V);
	cosTheta = smoothstep(roughness,1.0 , cosTheta);
	vec3 endColor =  (ambient + diffuse + specular);
	vec3 fresnel  = fresnelSchlick(cosTheta, vec3(F0));
	//fresnel = fresnelSchlickRoughness(max(cosTheta, 0.0), vec3(F0), roughness); 
	vec3 irradiance = texture(irradianceMap, reflect(-V, normal)).xyz;
	//return vec3(fresnel);
	return mix(endColor, irradiance , fresnel.x);
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
    vec3 ambient  = 0.1  * vec3(texture(gDiffuse, vTexCoord)) * light.color;
    vec3 diffuse  = diff * vec3(texture(gDiffuse, vTexCoord)) * light.color;
    vec3 specular = 0.5 * spec * light.color;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
    
}

void main()
{	if(renderMode == 0)
	{
		if(texture(gDiffuse, vTexCoord).a == 0.0)
		{
			discard;
		};
		
	vec3 vPosition = texture(gPosition, vTexCoord).xyz;
	vec3 vViewDir	= normalize(uCameraPosition - vPosition);

	// Properties
	vec3 norm = normalize(texture(gNormal, vTexCoord).xyz);
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

	//oColor = texture(gDiffuse, vTexCoord);
	oColor = vec4(result, 1.0);
	}
	else if(renderMode ==1)
	{
		oColor = texture(gNormal, vTexCoord);
	}

	else if(renderMode ==2)
	{
		oColor.xyz = vec3(LinearizeDepth(vTexCoord));
		//oColor.xyz = normalize(oColor.xyz);
		oColor.w =1.0;
	}

	else if (renderMode == 3){
		oColor = texture(gPosition, vTexCoord);
	}
	else if (renderMode == 4)
	{
		if(texture(gDiffuse, vTexCoord).a == 0.0)
		{
			discard;
		};

		vec3 vPosition = texture(gPosition, vTexCoord).xyz;
		vec3 vViewDir	= normalize(uCameraPosition - vPosition);
		vec3 viewDir = normalize(vViewDir - vPosition);
		vec3 V	= normalize(vPosition - uCameraPosition);
		vec3 N = normalize(texture(gNormal, vTexCoord).xyz);

		vec3 viewPos = vec3(viewMatrix * vec4(vPosition, 1.0));
		oColor.rgb = texture(environmentMap, reflect(-vViewDir, N)).xyz;
		oColor.a = 1.0;
	}
	//oColor = vec4(1.0,0.0,0.0,1.0);
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.