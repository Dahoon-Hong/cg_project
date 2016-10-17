#version 130

// inputs from vertex shader
in vec2 tc;	// used for texture coordinate visualization
in vec4 epos;
in vec3 norm;
//in mat3 TBN;
in mat3 TBN;

// output of the fragment shader
out vec4 fragColor;


// shader's global variables, called the uniform variables
uniform mat4	view_matrix, model_matrix;
uniform vec4	light_position, Ia, Id, Is;	// light
uniform vec4	Ka, Kd, Ks;					// material properties
uniform float	shininess;

//bump mapping
uniform bool is_bump;
uniform bool is_alpha;
uniform sampler2D TEX;
uniform sampler2D BUMP;
uniform sampler2D ALPHA;

void main()
{

	vec4 lpos = view_matrix*light_position;

	vec3 n = normalize(norm);	// norm interpolated via rasterizer should be normalized again here
	vec3 p = epos.xyz;			// 3D position of this fragment
	vec3 l = normalize((lpos.xyz - (lpos.a == 0.0 ? vec3(0) : p)));	// lpos.a==0 means directional light
	vec3 v = normalize(-p);		// eye-epos = vec3(0)-epos
	vec3 h = normalize(l + v);	// the halfway vector

	if (is_bump == false){
		vec4 Ira = Ka*Ia;	// ambient reflection
		vec4 Ird = max(Kd*dot(l, n)*Id, 0.0);					// diffuse reflection
		vec4 Irs = max(Ks*pow(dot(h, n), shininess)*Is, 0.0);	// specular reflection

		
		//alpha blending
		float a = 1;
		vec4 alpha = vec4(1, 1, 1, 1);
		//QA
		if (is_alpha == true){
			alpha = texture2D(ALPHA, tc).aaaa;
			a = alpha.a;
		}
		//res colors
		fragColor = texture2D(TEX,tc)*(Ira + Ird + Irs);
		fragColor.a = a;
	}
	else{
		n = normalize(TBN*(vec3(texture(BUMP, tc).rgb * 2.0 - 1.0)));

		vec4 Ira = Ka*Ia;	// ambient reflection
		vec4 Ird = max(Kd*dot(l, n)*Id, 0.0);					// diffuse reflection
		vec4 Irs = max(Ks*pow(dot(h, n), shininess)*Is, 0.0);	// specular reflection

		//res color
		fragColor = texture2D(TEX, tc) * (Ira + Ird + Irs);
	}
}