#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cglm/cglm.h>

#include <cglm/types-struct.h>
#include <stdio.h>
#include <math.h>

#include "rectangle.h"

// Fill the vertex array with co-ordinates
// // This function draws a square which will then be used to make rectangles through scaling.
void fillRectangleVertexArray(Vertex rectangleVertices[RECTANGLE_VERTEX])
{
	//coordinates xyzw
	static vec4s Coords[RECTANGLE_VERTEX] = {
		{ 0.0, 0.0, 0.0, 1.0 },
	    { 1.0, 0.0, 0.0, 1.0 },
	    { 0.0, 1.0, 0.0, 1.0 },
	    { 1.0, 1.0, 0.0, 1.0 }
	};
	
	// vertex normals
	static vec3s Normal[RECTANGLE_VERTEX] = {
		{ 0.0, 0.0, 1.0},
	    { 0.0, 0.0, 1.0},
	    { 0.0, 0.0, 1.0},
	    { 0.0, 0.0, 1.0}
	};
	
	// vertex texture coordinates
	static vec2s TexCoords[RECTANGLE_VERTEX] = {
		{0.0, 0.0},
		{6.0, 0.0},
		{0.0, 6.0},
		{6.0, 6.0}
	};

	int k;
	
    for (k = 0; k < RECTANGLE_VERTEX; k++)
    {
       rectangleVertices[k].coords = Coords[k];
       rectangleVertices[k].normal = Normal[k];
	   rectangleVertices[k].texCoords = TexCoords[k];
    }
}

// This function draws 2 squares which will then be used to make rectangles through scaling.
void fillDoubleRectangleVertexArray(Vertex doubleRectangleVertices[DOUBLE_RECTANGLE_VERTEX])
{
	//coordinate xyzw
	static vec4s Coords[DOUBLE_RECTANGLE_VERTEX] = {
		//face 1
		{ 0.0, 0.0, 0.0, 1.0 },
	    { 1.0, 0.0, 0.0, 1.0 },
	    { 0.0, 1.0, 0.0, 1.0 },
	    { 1.0, 1.0, 0.0, 1.0 },
	    //face 2
    	{ 1.0, 0.0, 0.0, 1.0 },
    	{ 1.0, 0.0, -1.0, 1.0 },
    	{ 1.0, 1.0, 0.0, 1.0},
    	{ 1.0, 1.0, -1.0, 1.0},
		};
	
	// vertex normals
	static vec3s Normal[DOUBLE_RECTANGLE_VERTEX] = {
		//face 1
		{ 0.0, 0.0, 1.0},
	    { 0.0, 0.0, 1.0},
	    { 0.0, 0.0, 1.0},
	    { 0.0, 0.0, 1.0},
	    //face 2
	    { 1.0, 0.0, 0.0},
	    { 1.0, 0.0, 0.0},
	    { 1.0, 0.0, 0.0},
	    { 1.0, 0.0, 0.0}
	};
	
	// vertex texture coordinates
	static vec2s TexCoords[DOUBLE_RECTANGLE_VERTEX] = {
		//face 1
		{0.0, 0.0},
		{4.0, 0.0},
		{0.0, 6.0},
		{4.0, 6.0},
		//face 2
		{4.0, 0.0},
		{8.0, 0.0},
		{4.0, 6.0},
		{8.0, 6.0}
	};

	int k;
	
    for (k = 0; k < DOUBLE_RECTANGLE_VERTEX; k++)
    {
       doubleRectangleVertices[k].coords = Coords[k];
       doubleRectangleVertices[k].normal = Normal[k];
	   doubleRectangleVertices[k].texCoords = TexCoords[k];
    }
}
