#pragma once
#include "cgmath.h"
//*******************************************************************
// common structures
struct camera
{
	vec3	eye	= vec3(0, 0, 150);
	vec3	at 	= vec3(0, 0, 0);
	vec3	up 	= vec3(0, 1, 0);
	mat4	view_matrix = mat4::lookAt(eye, at, up);

	float	fovy	= PI / 4.0f; // must be in radian
	float	aspect_ratio;
	float	dNear	= 1.0f;
	float	dFar	= 10000.0f;
	mat4	projection_matrix;
};

struct trackball
{
	bool	bTracking;		// is command rotate
	bool	bZoom;			// is command zoom
	bool	bPanning;		// is command pan
	float	scale;			// controls how much rotation is applied
	mat4	view_matrix0;	// initial view matrix, current view matrix
	vec2	m0;				// the last mouse position
	trackball( float rot_scale=1.0f ):bTracking(false),scale(rot_scale),bZoom(false),bPanning(false){}

	void end(){ bTracking = false; bZoom = false; bPanning = false; }
	void begin( const mat4& view_matrix, float x, float y)
	{
		m0 = vec2(x,y)*2.0f-1.0f;	// convert (x,y) in [0,1] to [-1,1]
		view_matrix0 = view_matrix;	// save current view matrix
	}

	void update( float x, float y,camera& cam)
	{
		if (bTracking){
			// project a 2D mouse position to a unit sphere
			static const vec3 p0 = vec3(0,0,1.0f);							// original position of the camera
			vec3 p1 = vec3(x*2.0f - 1.0f - m0.x, -m0.y + y*2.0f - 1.0f, 0);			// calculate displacement with vertical y-swapping
			if (!bTracking || length(p1) < 0.0001f) return;			// ignore subtle movement
			p1 *= scale;													// apply scaling
			p1 = vec3(p1.x, p1.y, sqrtf(max(0, 1.0f - length(p1)))).normalize();	// adjust z to make unit sphere

			// find rotation axis and angle (with inverse view rotation to the world coordinate)
			vec3 n = p0.cross(p1)*((mat3)view_matrix0);
			//float angle = -asin(min(n.length(), 0.999f))/10;
			cam.eye = (mat4::translate(cam.at) * mat4::rotate(cam.up, -p1.x*3.14f) * mat4::translate(-cam.at)) * vec4(cam.eye, 1);
			cam.eye = (mat4::translate(cam.at) * mat4::rotate((cross(cam.up, cam.at - cam.eye)).normalize(), p1.y*3.14f) * mat4::translate(-cam.at)) * vec4(cam.eye, 1);
			cam.up = ((mat4::translate(cam.at) * mat4::rotate((cross(cam.up, cam.at - cam.eye)).normalize(), p1.y*3.14f) * mat4::translate(-cam.at)) * vec4(cam.up, 0)).normalize();
		}
		else if (bZoom){
			vec3 p1 = vec3(x*2.0f - 1.0f - m0.x, -m0.y + y*2.0f - 1.0f, 0);
			p1 *= 10;
			cam.eye = cam.at + (cam.at - cam.eye)*((p1.y)/20-1);
		}
		else if (bPanning){
			vec3 p1 = vec3(x*2.0f - 1.0f - m0.x, m0.y - y*2.0f + 1.0f, 0);
			float length = 2 * vec3(cam.eye - cam.at).length() * tanf(cam.fovy / 2);
			cam.at = cam.at + (cam.up * -p1.y * length * 1024/576) + ((cross(cam.up, cam.at - cam.eye)).normalize() * p1.x * length);
		}
		//update m0
		m0 = vec2(x, y)*2.0f - 1.0f;
		return;
	}
};