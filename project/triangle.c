#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cglm/cglm.h>

#include <cglm/types-struct.h>
#include <stdio.h>
#include <math.h>

#include "triangle.h"

// Fill the vertex array with co-ordinates
void fillTriangleVertexArray(Vertex triangleVertices[TRIANGLE_VERTEX])
{
	//coordinate xyzw
	static vec4s Coords[TRIANGLE_VERTEX] = {
		{ 0.0, 0.0, 0.0, 1.0 },
	    { 2.0, 0.0, 0.0, 1.0 },
	    { 1.0, 1.0, 0.0, 1.0 }
	};
	
	static vec3s Normal[TRIANGLE_VERTEX] = {
		{ 0.0, 0.0, 1.0},
	    { 0.0, 0.0, 1.0},
	    { 0.0, 0.0, 1.0}
	};
	
	static vec2s TexCoords[TRIANGLE_VERTEX]	= {
		{0.0, 0.0},
		{3.0, 0.0},
		{1.5, 1.5}
	};

	int k;
	
    for (k = 0; k < TRIANGLE_VERTEX; k++ ) 
	{
       triangleVertices[k].coords = Coords[k];
       triangleVertices[k].normal = Normal[k];
	   triangleVertices[k].texCoords = TexCoords[k];
    }
}
