#ifndef RECTANGLE_H_INCLUDED
#define RECTANGLE_H_INCLUDED

#include "vertex.h"

#define RECTANGLE_VERTEX 4 // Number of vertex.
#define DOUBLE_RECTANGLE_VERTEX 8 // Number of vertex.

void fillRectangleVertexArray(Vertex rectangleVertices[RECTANGLE_VERTEX]); // This function draws a square which will then be used to make rectangles through scaling.
void fillDoubleRectangleVertexArray(Vertex doubleRectangleVertices[DOUBLE_RECTANGLE_VERTEX]); // This function draws 2 squares which will then be used to make rectangles through scaling.

#endif
