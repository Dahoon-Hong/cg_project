#include "cgmath.h"		// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility
#include "trackball.h"
#include <stdio.h>

//*******************************************************************
// include stb_image with the implementation preprocessor definition
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//*****************************
#define NUM_OF_TEXTURE 13
#define DWA 0
#define SUN 1
#define MER 2
#define VEN 3
#define EAR 4
#define MAR 5
#define JUP 6
#define SAT 7
#define URA 8
#define NEP 9
#define MOO 10
#define COM 11
#define RING 12

#define TEX 0
#define BUMP 1
#define BOOL 2
//*******************************************************************
// global constants
struct light_t
{
	vec4	position	= vec4(0.0f, 0.0f, 0.0f, 1.0f);   // directional light
	vec4	ambient		= vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse		= vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular	= vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

struct material_t
{
	vec4	ambient		= vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse		= vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular	= vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float	shininess	= 1000.0f;
	//float shininess = 0;
};

//*******************************************************************
// global constants
static const char*	window_name = "cg_A4_SolarSystem";
static const char*	vert_shader_path = "../bin/shaders/sphe.vert";
static const char*	frag_shader_path = "../bin/shaders/sphe.frag";
static const char*	texture_path[NUM_OF_TEXTURE] = {  "../bin/textures/moon.jpg" 
													, "../bin/textures/sun.jpg"
													, "../bin/textures/mercury.jpg"
													, "../bin/textures/venus.jpg"
													, "../bin/textures/earth.jpg"
													, "../bin/textures/mars.jpg"
													, "../bin/textures/jupiter.jpg"
													, "../bin/textures/saturn.jpg"
													, "../bin/textures/uranus.jpg"
													, "../bin/textures/neptune.jpg"
													, "../bin/textures/moon.jpg"
													, "../bin/textures/moon.jpg"
													, "../bin/textures/saturn-ring.jpg"
													};
static const char*	bump_path[NUM_OF_TEXTURE] = {	  "../bin/textures/moon-normal.jpg"
													, "../bin/textures/sun.jpg"
													, "../bin/textures/mercury-normal.jpg"
													, "../bin/textures/venus-normal.jpg"
													, "../bin/textures/earth-normal.jpg"
													, "../bin/textures/mars-normal.jpg"
													, "../bin/textures/jupiter.jpg"
													, "../bin/textures/saturn.jpg"
													, "../bin/textures/uranus.jpg"
													, "../bin/textures/neptune.jpg"
													, "../bin/textures/moon-normal.jpg"
													, "../bin/textures/moon-normal.jpg"
													, "../bin/textures/saturn-ring-alpha.jpg"
													};
static const char* alpha = "../bin/textures/saturn-ring-alpha.jpg";

static const bool	is_bump[NUM_OF_TEXTURE] = {1,0,1,1,1,1,0,0,0,0,1,1,0};
static const uint	MIN_TESS = 3;		// minimum tessellation factor (down to a triangle)
static const uint	MAX_TESS = 256;		// maximum tessellation factor (up to 256 triangles)
uint				NUM_TESS = 72;		// initial tessellation factor of the sphere as a polygon
static const uint	NUM_INSTANCE = 9;

//*******************************************************************
// window objects
GLFWwindow*	window		= nullptr;
ivec2		window_size = ivec2( 1024, 576 );	// initial window size

//*******************************************************************
// OpenGL objects
GLuint	program			= 0;	// ID holder for GPU program
GLuint	vertex_buffer	= 0;	// ID holder for vertex buffer
GLuint	index_buffer	= 0;	// ID holder for index buffer
GLuint	ring_vertex_buffer = 0;	// ID holder for vertex buffer
GLuint	ring_index_buffer = 0;	// ID holder for index buffer
GLuint	textures[NUM_OF_TEXTURE];
GLuint	bump[NUM_OF_TEXTURE];
GLuint	alphaObj;
//*******************************************************************
// global variables
int		frame	= 0;				// index of rendering frames
vec4    solid_color = vec4(1.0f, 0.5f, 0.5f, 1.0f);
float	radius = 1.0f;
int		bUseSolidColor = 0;
bool	bUseIndexBuffer = true;
bool	bWireframe = false;			// this is the default
float	scale_factor[10]	= { 0.0f, 14.0f, 1.6f, 2.0f, 2.4f, 3.0f, 8.0f, 6.0f, 2.0f,2.5f };
vec3	locRotAngle[10] = { vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 1, 4), vec3(0, 0, 1), 
							vec3(0, 0, 1), vec3(0, 1, 4), vec3(0, 1, 1), vec3(0, 1, 3), vec3(0, 0, 1)	};
vec3	gloRotAngle[10] = { vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1), vec3(0,0, 1),
							vec3(0, 0, 1), vec3(0, 0.05f, 1), vec3(0, 0.06f, 1), vec3(0, 0, 1), vec3(0,0, 1)	};
int		dwarfNum[10]	= { 0, 1, 1, 1, 4, 3, 0, 1, 2 };

bool	bShift	= false;
bool	bCtrl	= false;
float	lastTime = 0;
float	curTime = 0;
bool	bRotate = true;
uint	scale_ring = 10;
uint	_inner = 1 * scale_ring;
uint	_outer = 2 * scale_ring;

//*******************************************************************
// holder of vertices and indices
std::vector<vertex>	vertex_list;	// host-side vertices
std::vector<vertex> ring_vertex_list;
std::vector<uint>	index_list;		// host-side indices
std::vector<uint>	ring_index_list;		// host-side indices

//*******************************************************************
// scene objects
camera		cam;
trackball	tb;
light_t		light;
material_t	material;

//*******************************************************************
void update()
{
	cam.aspect_ratio = window_size.x / float(window_size.y);
	cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect_ratio, cam.dNear, cam.dFar);
	cam.view_matrix = mat4::lookAt(cam.eye, cam.at, cam.up);

	// update uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation(program, "view_matrix");			if (uloc>-1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program, "projection_matrix");	if (uloc>-1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);

	// setup light properties
	glUniform4fv(glGetUniformLocation(program, "light_position"), 1, light.position);//light.position);
	glUniform4fv(glGetUniformLocation(program, "Ia"), 1, light.ambient);
	glUniform4fv(glGetUniformLocation(program, "Id"), 1, light.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Is"), 1, light.specular);

	// setup material properties
	glUniform4fv(glGetUniformLocation(program, "Ka"), 1, material.ambient);
	glUniform4fv(glGetUniformLocation(program, "Kd"), 1, material.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Ks"), 1, material.specular);
	glUniform1f(glGetUniformLocation(program, "shininess"), material.shininess);

}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// notify GL that we use our own program
	glUseProgram( program );

	// bind vertex attributes to your shader program
	const char*	vertex_attrib[] = { "position", "normal", "texcoord" };

	size_t		attrib_size[] = { sizeof(vertex().pos), sizeof(vertex().norm), sizeof(vertex().tex), sizeof(vec3), sizeof(vec3)};
	GLuint loc;
	size_t byte_offset = 0;
	for( size_t k=0, kn=std::extent<decltype(vertex_attrib)>::value, byte_offset=0; k<kn; k++, byte_offset+=attrib_size[k-1] )
	{
		loc = glGetAttribLocation( program, vertex_attrib[k] ); if(loc>=kn) continue;
		glEnableVertexAttribArray( loc );
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(loc, attrib_size[k] / sizeof(GLfloat), GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*)byte_offset);

	}
	
	GLint isA, isB, tx, bp;
	isA = glGetUniformLocation(program, "is_alpha");	if(isA > -1) glUniform1i(isA, false);

	if (bRotate){
		curTime = float(glfwGetTime());
		lastTime = curTime;
	}
	else{
		glfwSetTime(double(lastTime));
	}
	float t = curTime / 2;

	
	for (int k = 0, kn = int(NUM_INSTANCE); k <= kn; k++){
		if (k == 6)
			continue;

		float scale = scale_factor[k + 1];
		float kt = (10 - k) * t / 4 + scale_factor[k];
		float move = float(k*(k + 2)) * 3 + scale_factor[k] * 2;

		mat4 model_matrix =
			mat4::rotate(gloRotAngle[k].normalize(), kt) * //global rotation
			mat4::translate(move, 0.0f, 0.0f) *
			mat4::rotate(locRotAngle[k].normalize(), kt * 2) *  //local rotation
			mat4::scale(scale, scale, scale);

		if (k == 0){
			glUniform1f(glGetUniformLocation(program, "shininess"), 0.0f);
			material.shininess = 100.0f;
		}
		else{
			glUniform1f(glGetUniformLocation(program, "shininess"), material.shininess);
			material.shininess *= 1.2f;
		}


		//del
		isB = glGetUniformLocation(program, "is_bump"); if (isB > -1) glUniform1i(isB, is_bump[k + 1]);


		//del
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, bump[k + 1]);
		bp = glGetUniformLocation(program, "BUMP");	if (bp>-1)	glUniform1i(bp, 1);	 // GL_TEXTURE0


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures[k + 1]);
		tx = glGetUniformLocation(program, "TEX"); if (tx>-1)	glUniform1i(tx, 0);	 // GL_TEXTURE0

		

		glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, model_matrix);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glDrawElements(GL_TRIANGLES, index_list.size(), GL_UNSIGNED_INT, nullptr);

		
		//dwarf
		for (int k1 = 0, kn1 = dwarfNum[k]; k1 < kn1; k1++){
			float move1 = ( 3) + scale_factor[k+1];
			mat4 model_matrix =
				mat4::rotate(gloRotAngle[k].normalize(), kt) * //revolution to solar
				mat4::translate(move, 0.0f, 0.0f) *
				mat4::rotate(gloRotAngle[k1].normalize(), float(k1*k1+kt)) * //revolution to planet
				mat4::translate(float(move1*sin(k1)), float(move1*cos(k1)), 0.0f) *
				mat4::rotate(locRotAngle[k1].normalize(), kt * 2) *  //rotation
				mat4::scale(scale *1.3f / pow(scale, 2.0f), scale*1.3f / pow(scale, 2.0f), scale * 1.3f / pow(scale, 2.0f)); //scale

			//del
			isB = glGetUniformLocation(program, "is_bump"); if (isB > -1) glUniform1i(isB, is_bump[MOO]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[MOO]);
			tx = glGetUniformLocation(program, "TEX"); if (tx>-1) glUniform1i(tx, TEX);	 // GL_TEXTURE0

			//del
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, bump[MOO]);
			bp = glGetUniformLocation(program, "BUMP");	if (bp>-1) glUniform1i(bp, BUMP);	 // GL_TEXTURE1

			glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, model_matrix);
			if (index_buffer) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
			glDrawElements(GL_TRIANGLES, index_list.size(), GL_UNSIGNED_INT, nullptr);

		}
		
	}

	//comet
	float a = 4.5;
	float kt = a * t / 5;
	float move = (a*(a + 2) * 3);
	float scale = 1.5f;
	mat4 model_matrix =
		mat4::translate(cos(kt)*(a*(a + 2) * 15) + 250, sin(kt)*(a*(a + 2) * 2), 0.0f) *
		mat4::rotate(vec3(0, 0, 1).normalize(), kt)  *		
		mat4::scale(scale,scale,scale);

	glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, model_matrix);
	if (index_buffer) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glDrawElements(GL_TRIANGLES, index_list.size(), GL_UNSIGNED_INT, nullptr);

	//del
	glUniform1i(glGetUniformLocation(program, "is_bump"), is_bump[MOO]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[MOO]);
	glUniform1i(glGetUniformLocation(program, "TEX"), TEX);	 // GL_TEXTURE0

	//del
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bump[MOO]);
	glUniform1i(glGetUniformLocation(program, "BUMP"), BUMP);	 // GL_TEXTURE0

	//saturn
	int k = 6;
	scale = scale_factor[k + 1];
	kt = (10 - k) * t / 4 + scale_factor[k];
	move = float(k*(k + 2)) * 3;


	model_matrix =
		mat4::rotate(gloRotAngle[k].normalize(), kt) * //global rotation
		mat4::translate(move, 0.0f, 0.0f) *
		mat4::rotate(locRotAngle[k].normalize(), kt * 2) *  //local rotation
		mat4::scale(scale, scale, scale);

	//del
	glUniform1i(glGetUniformLocation(program, "is_bump"), is_bump[SAT]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[SAT]);
	glUniform1i(glGetUniformLocation(program, "TEX"), 0);	 // GL_TEXTURE0

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bump[SAT]);
	glUniform1i(glGetUniformLocation(program, "BUMP"), 1);	 // GL_TEXTURE0

	glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, model_matrix);
	if (index_buffer) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glDrawElements(GL_TRIANGLES, index_list.size(), GL_UNSIGNED_INT, nullptr);

	//ring
	model_matrix =
		mat4::rotate(gloRotAngle[k].normalize(), kt) * //global rotation
		mat4::translate(move, 0.0f, 0.0f) *
		//mat4::rotate(locRotAngle[k].normalize(), kt * 2) *  //local rotation
		mat4::rotate(vec3(0, 1, 1).normalize(), 30)*
		mat4::scale(scale, scale, scale);

	glDisable(loc);
	for (size_t k1 = 0, kn1 = std::extent<decltype(vertex_attrib)>::value, byte_offset = 0; k1<kn1; k1++, byte_offset += attrib_size[k1 - 1])
	{
		loc = glGetAttribLocation(program, vertex_attrib[k1]); if (loc >= kn1) continue;
		glEnableVertexAttribArray(loc);
		glBindBuffer(GL_ARRAY_BUFFER, ring_vertex_buffer);
		glVertexAttribPointer(loc, attrib_size[k1] / sizeof(GLfloat), GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*)byte_offset);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//del
	glUniform1i(glGetUniformLocation(program, "is_bump"), is_bump[RING]);
	glUniform1i(glGetUniformLocation(program, "is_alpha"), true);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, alphaObj);
	glUniform1i(glGetUniformLocation(program, "ALPHA"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[RING]);
	glUniform1i(glGetUniformLocation(program, "TEX"), 0);	


	glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, model_matrix);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ring_index_buffer);
	glDrawElements(GL_TRIANGLES, ring_index_list.size(), GL_UNSIGNED_INT, nullptr);

	glDisable(GL_BLEND);

	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	glViewport( 0, 0, width, height );
}
void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf( "- press 'd' to toggle among three texture coordinates ( (tc.xy, 0, 1), (tc.xxx, 1), (tc.yyy, 1) )\n" );
	printf( "- press '+/-' to increase/decrease tessellation factor (min=%d, max=%d)\n", MIN_TESS, MAX_TESS );
	printf( "- press 'w' to toggle wireframe\n" );
	printf( "- press 'shift' + 'left button' or 'right button' to zooming \n");
	printf( "- press 'control' + 'left button' or 'middle button' to panning \n");
	printf( "- press 'left button' to rotate viewing angle \n");
	printf( "- press 'i' to initiate camera view matirx \n");
	printf( "- press 's' to stop animation \n");
	printf("\n"); 
}
void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	void update_vertex_buffer(uint N);	// forward declaration
	void update_sphere_vertices(uint N);	// forward declaration
	void update_ring_vertices(uint inner, uint outer, uint NS);
	void update_ring_vertex_buffer(uint inner, uint outer, uint N);
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_RIGHT_SHIFT || key == GLFW_KEY_LEFT_SHIFT)	bShift = true;
		else if (key == GLFW_KEY_RIGHT_CONTROL || key == GLFW_KEY_LEFT_CONTROL)	bCtrl = true;
		else if (key == GLFW_KEY_KP_ADD || (key == GLFW_KEY_EQUAL && (mods&GLFW_MOD_SHIFT)))
		{
			if (NUM_TESS >= MAX_TESS) return;
			update_sphere_vertices(++NUM_TESS);
			update_vertex_buffer(NUM_TESS);
			update_ring_vertices(_inner, _outer, NUM_TESS);
			update_ring_vertex_buffer(_inner, _outer, NUM_TESS);
			printf("> NUM_TESS = % -4d\r", NUM_TESS);
		}
		else if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_MINUS)
		{
			if (NUM_TESS <= MIN_TESS) return;
			update_sphere_vertices(--NUM_TESS);
			update_vertex_buffer(NUM_TESS);
			update_ring_vertices(_inner,_outer,NUM_TESS);
			update_ring_vertex_buffer(_inner, _outer, NUM_TESS);
			printf("> NUM_TESS = % -4d\r", NUM_TESS);
		}
		else if (key == GLFW_KEY_W)
		{
			bWireframe = !bWireframe;
			glPolygonMode(GL_FRONT_AND_BACK, bWireframe ? GL_LINE : GL_FILL);
			printf("> using %s mode\n", bWireframe ? "wireframe" : "solid");
		}
		else if (key == GLFW_KEY_D)
		{
			++bUseSolidColor %= 3;
			switch ((int)bUseSolidColor){
			case 0:
				printf("> using %s\n", "texture coordinate(vec4(tc.xy,0,1)) as color");
				break;
			case 1:
				printf("> using %s\n", "texture coordinate(vec4(tc.xxx,1)) as color");
				break;
			case 2:
				printf("> using %s\n", "texture coordinate(vec4(tc.yyy,1)) as color");
				break;
			}
		}
		else if (key == GLFW_KEY_I)
		{
			printf("> Init camera view_matrix\n");
			cam = camera();
		}
		else if (key == GLFW_KEY_S){
			bRotate = !bRotate;
			if (bRotate)
				printf("> rotation is activated\n");
			else
				printf("> rotation is deactivated\n");
		}
	}
	else if (action == GLFW_RELEASE){
		if (key == GLFW_KEY_RIGHT_SHIFT || key == GLFW_KEY_LEFT_SHIFT)	bShift = false;
		else if (key == GLFW_KEY_RIGHT_CONTROL || key == GLFW_KEY_LEFT_CONTROL)	bCtrl = false;
	}
	
}

void mouse( GLFWwindow* window, int button, int action, int mods )
{
	dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
	vec2 npos = vec2(float(pos.x) / float(window_size.x - 1), float(pos.y) / float(window_size.y - 1));

	if(button==GLFW_MOUSE_BUTTON_LEFT)
	{
		if (bShift)	tb.bZoom = true;	else if (bCtrl)	tb.bPanning = true;		else tb.bTracking = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		tb.bZoom = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE){
		tb.bPanning = true;
	}
	if (action == GLFW_PRESS)			tb.begin(cam.view_matrix, npos.x, npos.y);
	else if (action == GLFW_RELEASE)	tb.end();

}

void motion( GLFWwindow* window, double x, double y )
{
	if (!(tb.bTracking || tb.bZoom || tb.bPanning) ) return;
	vec2 npos = vec2(float(x) / float(window_size.x - 1), float(y) / float(window_size.y - 1));
	tb.update(npos.x, npos.y, cam);
	return;
}

void update_vertex_buffer( uint N )
{
	// clear and create new buffers
	if(vertex_buffer)	glDeleteBuffers( 1, &vertex_buffer );	vertex_buffer = 0;
	if(index_buffer)	glDeleteBuffers( 1, &index_buffer );	index_buffer = 0;
	
	
	// check exceptions
	if(vertex_list.empty()){ printf("[error] vertex_list is empty.\n"); return; }
	
	// create buffers
	index_list.clear();
	for( uint k=0; k < N; k++ )
	{
		for (uint i = 0; i < N; i++){
			index_list.push_back( (k * (N+1)) + i);	// the origin
			index_list.push_back(((k + 1) * (N + 1)) + i);
			index_list.push_back(((k + 1) * (N + 1)) + i + 1);
		}
	}
	for (uint k = N; k > 0; k--)
	{
		for (uint i = N; i > 0; i--){
			index_list.push_back((k * (N + 1)) + i);
			index_list.push_back(((k - 1) * (N + 1)) + i);
			index_list.push_back(((k - 1) * (N + 1)) + i - 1);
		}
	}
		// generation of vertex buffer: use vertex_list as it is
	glGenBuffers( 1, &vertex_buffer );
	glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertex)*vertex_list.size(), &vertex_list[0], GL_STATIC_DRAW);

	// geneation of index buffer
	glGenBuffers( 1, &index_buffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, index_buffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*index_list.size(), &index_list[0], GL_STATIC_DRAW );
}
void update_sphere_vertices( uint N )
{
	vertex_list.clear();
	// define the position of four corner vertices
	for (uint k = 0; k <= N; k++)
	{
		float theta = PI / float(N) * float(k);
		for (uint i = 0; i <= N; i++){
			float pi = PI * 2.0f / float(N) * float(i);
			float cp = cos(pi), sp = sin(pi);
			float ct = cos(theta), st = sin(theta);
			vertex_list.push_back({ vec3(st*cp, st*sp, ct), vec3(st*cp, st*sp, ct),
									vec2(pi / (2 * PI), 1 - theta / PI) });
		}
	}
}
void update_ring_vertex_buffer(uint inner, uint outer, uint N){
	if (ring_vertex_buffer)	glDeleteBuffers(1, &ring_vertex_buffer);	ring_vertex_buffer = 0;
	if (ring_index_buffer)	glDeleteBuffers(1, &ring_index_buffer);	ring_index_buffer = 0;
	if (ring_vertex_list.empty()){ printf("[error] ring_vertex_list is empty.\n"); return; }
	ring_index_list.clear();
	uint next = outer - inner + 1;

	for (uint k = 0; k < N; k++)
	{
		for (uint i = 0; i < next - 1; i++){
			ring_index_list.push_back((k * (next)) + i);
			ring_index_list.push_back(((k + 1) * (next)) + i);
			ring_index_list.push_back(((k + 1) * (next)) + i + 1);
		}
	}
	for (uint k = N; k > 0; k--)
	{

		for (uint i = next - 1; i > 0; i--){
			ring_index_list.push_back((k * (next)) + i);
			ring_index_list.push_back(((k - 1) * (next)) + i);
			ring_index_list.push_back(((k - 1) * (next)) + i - 1);
		}
	}
	for (uint k = N+1; k < 2*N+1; k++)
	{
		for (uint i = 0; i < next - 1; i++){
			ring_index_list.push_back((k * (next)) + i);
			ring_index_list.push_back(((k + 1) * (next)) + i + 1);
			ring_index_list.push_back(((k + 1) * (next)) + i);
		}
	}
	for (uint k = 2*N+1; k > N+1; k--)
	{
		for (uint i = next - 1; i > 0; i--){
			ring_index_list.push_back((k * (next)) + i);
			ring_index_list.push_back(((k - 1) * (next)) + i - 1);
			ring_index_list.push_back(((k - 1) * (next)) + i);
		}
	}

	// generation of vertex buffer: use vertex_list as it is
	glGenBuffers(1, &ring_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, ring_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*ring_vertex_list.size(), &ring_vertex_list[0], GL_STATIC_DRAW);

	// geneation of index buffer
	glGenBuffers(1, &ring_index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ring_index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*ring_index_list.size(), &ring_index_list[0], GL_STATIC_DRAW);
}
void update_ring_vertices(uint inner, uint outer, uint N){
	ring_vertex_list.clear();
	
	
	for (uint k = 0; k <= N; k++){
		float theta = 2 * PI / float(N) * float(k);
		for (uint r = inner; r <= outer; r++){
			float c = cos(theta), s = sin(theta);
			ring_vertex_list.push_back({ vec3(r*c *1.5f / scale_ring, r*s *1.5f / scale_ring, 0), vec3(0,0, -1).normalize(), vec2((float)(r - inner) / (outer - inner + 1), 0) });
		}
	}
	for (uint k = N+1; k <= 2*N+1; k++){
		float theta = 2 * PI / float(N) * float(k);
		for (uint r = inner; r <= outer; r++){
			float c = cos(theta), s = sin(theta);
			ring_vertex_list.push_back({ vec3(r*c *1.5f / scale_ring, r*s *1.5f / scale_ring, 0), vec3(0, 0, 1).normalize(), vec2((float)(r-inner)/(outer-inner +1), 0) });
		}
	}
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	//glLineWidth( 1.0f );
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests
	glEnable( GL_TEXTURE_2D );
	

	// create textures
	glGenTextures(NUM_OF_TEXTURE, textures);
	
	// load and flip an image
	glActiveTexture(GL_TEXTURE0);
	for (int k = 0; k < NUM_OF_TEXTURE; k++){
		int width, height, comp = 3;
		unsigned char* pimage0 = stbi_load(texture_path[k], &width, &height, &comp, 3); if (comp == 1) comp = 3; 
		int stride0 = width*comp, stride1 = (stride0 + 3)&(~3);	// 4-byte aligned stride
		unsigned char* pimage = (unsigned char*)malloc(sizeof(unsigned char)*stride1*height);
		for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y)*stride1, pimage0 + y*stride0, stride0); // vertical flip

		glBindTexture(GL_TEXTURE_2D, textures[k]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB , width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pimage);

		// allocate and create mipmap
		int mip_levels = get_mip_levels(window_size.x, window_size.y);
		for (int k1 = 1, w = width >> 1, h = height >> 1; k1 < mip_levels; k1++, w = max(1, w >> 1), h = max(1, h >> 1))
			glTexImage2D(GL_TEXTURE_2D, k1, GL_RGB , w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glGenerateMipmap(GL_TEXTURE_2D);
		delete[] pimage;
	}


	glGenTextures(NUM_OF_TEXTURE, bump);
	// load and flip an bump image
	glActiveTexture(GL_TEXTURE1);
	for (int k = 0; k < NUM_OF_TEXTURE; k++){
		int width, height, comp = 3;
		unsigned char* pimage0 = stbi_load(bump_path[k], &width, &height, &comp, 3); if (comp == 1) comp = 3;
		int stride0 = width*comp, stride1 = (stride0 + 3)&(~3);	// 4-byte aligned stride
		unsigned char* pimage = (unsigned char*)malloc(sizeof(unsigned char)*stride1*height);
		for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y)*stride1, pimage0 + y*stride0, stride0); // vertical flip

		glBindTexture(GL_TEXTURE_2D, bump[k]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pimage);
		// allocate and create mipmap
		int mip_levels = get_mip_levels(window_size.x, window_size.y);
		for (int k1 = 1, w = width >> 1, h = height >> 1; k1 < mip_levels; k1++, w = max(1, w >> 1), h = max(1, h >> 1))
			glTexImage2D(GL_TEXTURE_2D, k1, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		// configure texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		free(pimage);
	}

	//ring
	glGenTextures(1, &alphaObj);
	glActiveTexture(GL_TEXTURE2);
	int width, height, comp = 3;
	unsigned char* pimage0 = stbi_load(alpha, &width, &height, &comp, 3); 
	int stride0 = width*comp, stride1 = (stride0 + 3)&(~3);	// 4-byte aligned stride
	unsigned char* pimage = (unsigned char*)malloc(sizeof(unsigned char)*stride1*height);
	for (int y = 0; y < height; y++) memcpy(pimage + (height - 1 - y)*stride1, pimage0 + y*stride0, stride0); // vertical flip

	glBindTexture(GL_TEXTURE_2D, alphaObj);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pimage);
	// allocate and create mipmap
	int mip_levels = get_mip_levels(window_size.x, window_size.y);
	for (int k1 = 1, w = width >> 1, h = height >> 1; k1 < mip_levels; k1++, w = max(1, w >> 1), h = max(1, h >> 1))
		glTexImage2D(GL_TEXTURE_2D, k1, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, nullptr);
	glGenerateMipmap(GL_TEXTURE_2D);

	// configure texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	free(pimage);

	// define the position of four corner vertices
	update_sphere_vertices( NUM_TESS );
	update_ring_vertices(_inner, _outer, NUM_TESS);

	// create vertex buffer; called again when index buffering mode is toggled
	update_vertex_buffer( NUM_TESS );
	update_ring_vertex_buffer(_inner, _outer, NUM_TESS);

	return true;
}

void user_finalize()
{
	printf("- %s\n", "program is terminated");
}

void main( int argc, char* argv[] )
{
	// initialization
	if(!glfwInit()){ printf( "[error] failed in glfwInit()\n" ); return; }

	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movements

	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}
	
	// normal termination
	user_finalize();
	glfwDestroyWindow(window);
	glfwTerminate();
}
