#version 130

// vertex attributes
in vec3 position;
in vec3 normal;
in vec2 texcoord;
//in vec3 tangent;
//in vec3 bitangent;

// matrices
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform bool is_bump;

out vec3 norm;	// the second output
out vec4 epos;	// eye-coordinate position
out vec2 tc;	// the third output
out mat3 TBN;

void main()
{
	vec4 wpos = model_matrix * vec4(position, 1);
	epos = view_matrix * wpos;
	gl_Position = projection_matrix * (epos);
	
	tc = texcoord;
	norm = normalize(mat3(view_matrix*model_matrix)*normal);

	vec3 tangent, binormal;
	if (is_bump){
		vec3 c1 = cross(norm, vec3(0.0, 0.0, 1.0));
		vec3 c2 = cross(norm, vec3(0.0, 1.0, 0.0));

		if (length(c1) > length(c2))
		{
			tangent = c1;
		}
		else
		{
			tangent = c2;
		}

		tangent = normalize(tangent);

		binormal = cross(norm, tangent);
		binormal = normalize(binormal);
		

		TBN = mat3(tangent, binormal, norm);
	}
}


