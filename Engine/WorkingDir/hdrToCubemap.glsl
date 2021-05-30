#if defined(VERTEX)
layout (location = 0) in vec3 position;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out vec3 localPosition;

void main()
{
localPosition = position;
gl_Position = projectionMatrix * viewMatrix *  vec4(position, 1.0);
}

#elif defined(FRAGMENT)
in vec3 localPosition;
uniform sampler2D equirectangularMap;
out vec4 outColor;
const int pi = 3.14159265
const vec2 invAtan = vec2(1/pi * 2, 1/pi);
vec2 SampleSphericalMap(vec3 v)
{
vec2 uv = vec2(atan(v.z,v.x), asin(v.y));
uv *= invAtan;
uv += 0.5;
return uv;
}
void main()
{
vec2 uv = SampleSphericalMap(normalize(localPosition)); 
vec3 color = min(vec3(1000.0), texture(equirectangularMap, uv).rgb);
outColor = vec4(color, 1.0);
}
#endif
#endif