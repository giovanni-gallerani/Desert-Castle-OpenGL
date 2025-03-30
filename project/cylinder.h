#ifndef CYLINDER_H_INCLUDED
#define CYLINDER_H_INCLUDED

#include "vertex.h"

#define CYL_LONGS 6 // Number of longitudinal slices.
#define CYL_LATS 1 // Number of latitudinal slices.

void fillCylVertexArray(Vertex cylVertices[(CYL_LONGS + 1) * (CYL_LATS + 1)]);
void fillCylIndices(unsigned int cylIndices[CYL_LATS][2*(CYL_LONGS+1)]);
void fillCylCounts(int cylCounts[CYL_LATS]);
void fillCylOffsets(void* cylOffsets[CYL_LATS]);

void fillCylinder(Vertex cylVertices[(CYL_LONGS + 1) * (CYL_LATS + 1)],
	         unsigned int cylIndices[CYL_LATS][2*(CYL_LONGS+1)],
			 int cylCounts[CYL_LATS],
			 void* cylOffsets[CYL_LATS]);

#endif
