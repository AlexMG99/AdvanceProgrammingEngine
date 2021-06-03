#ifdef SKYBOX
#ifdef VERTEX
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;

uniform mat4 projection;
uniform mat4 view;

out vec3 WorldPos;
out vec3 normal;
void main()
{
	WorldPos = aPosition;

	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView * vec4(WorldPos, 1.0);

    normal = vec3(normal);

	gl_Position = clipPos;
}
#endif

#ifdef FRAGMENT
layout (location = 0) out vec4 gDifusse;		
layout (location = 1) out vec4 gNormal;		
//layout (location = 2) out vec4 gPosition;	

in vec3 WorldPos;
in vec3 normal;

uniform samplerCube environmentMap;

void main()
{		
    vec3 envColor = texture(environmentMap, WorldPos).rgb;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    gDifusse = vec4(envColor, 1.0);
    gNormal = vec4(normal, 1.0);
}

#endif
#endif
