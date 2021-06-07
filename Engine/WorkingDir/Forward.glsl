#ifdef FORWARD
#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;
//layout(location=3) in vec3 aTangent;
//layout(location=4) in vec3 aBitangent;


layout(binding = 0, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorlViewProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vPosition;
out vec3 vNormal;

void main()
{
	vTexCoord	= aTexCoord;
	vPosition	= vec3(uWorldMatrix * vec4(aPosition, 1.0));
	vNormal		= normalize(mat3(transpose(inverse(uWorldMatrix))) * aNormal);
	gl_Position = uWorlViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

struct Light
{
	unsigned int	type;
	vec3			color;
	vec3			direction;
	vec3			position;
};

layout(binding = 1, std140) uniform GlobalParams
{
	vec3			uCameraPosition;
	unsigned int	uLightCount;
	Light			uLight[16];
};

out vec4 oColor;
in vec2 vTexCoord;
in vec3 vPosition;
in vec3 vNormal;
uniform int hasTexture;
uniform vec3 color;
uniform sampler2D uTexture;
uniform samplerCube environmentMap; 
uniform samplerCube irradianceMap;
uniform float roughness= 1.0;
uniform float maxFresnel = 1.0; 
uniform vec3 F0 = vec3(0.5);

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
} 

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}  

vec3 CalculateDirLight(Light light, vec3 normal, vec3 V, vec3 color)
{
	vec3 lightDir = normalize(-light.direction);

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(V, reflectDir), 0.0), 32.0);
	

	// Combine
	vec3 ambient = 0.1 * texture(irradianceMap, reflect(-V, normal)).xyz * light.color;
	vec3 diffuse = diff * color * light.color;
	vec3 specular = 0.5 * spec * light.color;



	float cosTheta = dot(normal, V);
	cosTheta = smoothstep(roughness,maxFresnel , cosTheta);
	vec3 endColor =  (ambient + diffuse + specular);
	vec3 fresnel  = fresnelSchlick(cosTheta, vec3(F0));
	//fresnel = fresnelSchlickRoughness(max(cosTheta, 0.0), vec3(F0), roughness); 
	vec3 irradiance = texture(irradianceMap, reflect(-V, normal)).xyz;
	//return vec3(fresnel);
	return mix(endColor, irradiance , fresnel.x);
}



vec3 CalculatePointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 color)
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
    vec3 ambient  = 0.1  * color * light.color;
    vec3 diffuse  = diff * color * light.color;
    vec3 specular = 0.5 * spec * light.color;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
    
}

void main()
{
	vec3 result = vec3(0,0,0);
    vec3 objcolor = vec3 (0.0);
    vec3 V = normalize(uCameraPosition - vPosition);
    if(hasTexture == 1)
		objcolor = texture(uTexture, vTexCoord).xyz;
	else
		objcolor = color;

	for(int i = 0; i < uLightCount; i++)
	{
		if(uLight[i].type == 0)
			result += CalculateDirLight(uLight[i], vNormal, V, objcolor);
		else
			result += CalculatePointLight(uLight[i], vNormal, vPosition, V, objcolor);
	}

	oColor = vec4(result, 1.0);
}

#endif
#endif