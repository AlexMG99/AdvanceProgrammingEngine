///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef TEXTURED_GEOMETRY



#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;

uniform mat4 projectionMatrix;
uniform mat4 worldViewMatrix;

out Data
{
	vec3 positionViewspace;
	vec3 normalViewspace;
} VSOut;

void main()
{
	VSOut.positionViewspace = vec3(worldViewMatrix * vec4(position, 1.0));
	VSOut.normalViewspace = vec3(worldViewMatrix * vec4(normal, 1.0));
	gl_Position = projectionMatrix * vec4(VSOut.positionViewspace, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

uniform vec2 viewportSize;
uniform mat4 modelViewMatrix;
uniform mat4 viewMatrixInv;
uniform mat4 projectionMatrixInv;
uniform sampler2D reflectionMap;
uniform sampler2D refractionMap;
uniform sampler2D reflectionDepth;
uniform sampler2D refractionDepth;
uniform sampler2D normalMap;
uniform sampler2D dudvMap;

in Data
{
	vec3 positionViewspace;
	vec3 normalViewspace;
} FSIn;

out vec4 outColor;

vec3 fresnelSchlick(float cosTheta, vec3 F0){
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 reconstructPixelPosition(float depth)
{
	vec2 texCoords = gl_FragCoord.xy / viewportSize;
	vec3 positionNDC = vec3(texCoords * 2.0 - vec2(1.0), depth * 2.0 - 1.0);
	vec4 positionEyespace = projectionMatrixInv * vec4(positionNDC, 1.0);
	positionEyespace.xyz /= positionEyespace.w;
	return positionEyespace.xyz;
}

void main()
{
	vec3 N = normalize(FSIn.normalViewspace);
	vec3 V = normalize(-FSIn.positionViewspace);
	vec3 Pw = vec3(viewMatrixInv * vec4(FSIn.positionViewspace, 1.0));
	vec2 texCoord = gl_FragCoord.xy / viewportSize;

	const vec2 waveLength = vec2(2.0);
	const vec2 waveStrength = vec2(0.05);
	const float turbidityDistance = 10.0;

	vec2 distortion = (2.0 * texture(dudvMap, Pw.xz / waveLength).rg - vec2(1.0)) * waveStrength + waveStrength/7;

	// Distorted reflection and refraction
	vec2 reflectionTexCoord = vec2(texCoord.s, 1.0 - texCoord.t) + distortion;
	vec2 refractionTexCoord = texCoord + distortion;
	vec3 reflectionColor = texture(reflectionMap, reflectionTexCoord).rgb;
	vec3 refractionColor = texture(refractionMap, refractionTexCoord).rgb;

	// Water tint
	float distortedGroundDepth = texture(refractionDepth, refractionTexCoord).x;
	vec3 distortedGroundPosViewspace = reconstructPixelPosition(distortedGroundDepth);
	float distortedWaterDepth = FSIn.positionViewspace.z - distortedGroundPosViewspace.z;
	float tintFactor= clamp(distortedWaterDepth / turbidityDistance, 0.0, 1.0);
	vec3 waterColor = vec3(0.25, 0.4, 0.6);
	refractionColor = mix(refractionColor, waterColor, tintFactor);

	// Fresnel
	vec3 F0 = vec3(0.1);
	vec3 F = fresnelSchlick(max(0.0, dot(V, N)), F0);
	outColor.rgb = mix(refractionColor, reflectionColor, F);
	outColor.a = 1.0;
}

#endif
#endif


// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.
