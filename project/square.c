#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cglm/cglm.h>

#include <cglm/types-struct.h>
#include <stdio.h>
#include <math.h>

#include "square.h"

// Fill the vertex array with co-ordinates
void fillSquareVertexArray(Vertex squareVertices[SQUARE_VERTEX])
{
	//coordinates xyzw
	static vec4s Coords[SQUARE_VERTEX] = {
		{ 0.0, 0.0, 0.0, 1.0 },
	    { 1.0, 0.0, 0.0, 1.0 },
	    { 0.0, 1.0, 0.0, 1.0 },
	    { 1.0, 1.0, 0.0, 1.0 }
	};
	
	static vec3s Normal[SQUARE_VERTEX] = {
		{ 0.0, 0.0, 1.0},
	    { 0.0, 0.0, 1.0},
	    { 0.0, 0.0, 1.0},
	    { 0.0, 0.0, 1.0}
	};
	
	static vec2s TexCoords[SQUARE_VERTEX] = {
		{0.0, 0.0},
		{1.0, 0.0},
		{0.0, 1.0},
		{1.0, 1.0}
	};

	int k;
	
    for (k = 0; k < SQUARE_VERTEX; k++)
    {
       squareVertices[k].coords = Coords[k];
       squareVertices[k].normal = Normal[k];
	   squareVertices[k].texCoords = TexCoords[k];
    }
}
