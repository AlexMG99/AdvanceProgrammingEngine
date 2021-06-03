#ifdef VERTEX
	layout(location=0) in vec3 aPosition;
	layout(location=1) in vec3 aNormal;
	layout(location=2) in vec2 aTexCoord;
	
	out vec2 vTexCoord;
	
	void main()
	{
	  gl_Position = vec4(aPosition, 1.0);
	  vTexCoord = aTexCoord;
	}

#endif

#ifdef FRAGMENT
	in vec2 vTexCoord;

	out vec4 outColor;

	uniform sampler2D tex;
	void main()
	{
		outColor = texture(tex, vTexCoord);
	}

#endif
