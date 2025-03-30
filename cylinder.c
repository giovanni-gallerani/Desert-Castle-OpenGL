#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cglm/cglm.h>

#include <cglm/types-struct.h>
#include <stdio.h>
#include <math.h>

#include "cylinder.h"

// Fill the vertex array with co-ordinates of the sample points.
void fillCylVertexArray(Vertex cylVertices[(CYL_LONGS + 1) * (CYL_LATS + 1)])
{
   int i, j, k;

   k = 0;
   for (j = 0; j <= CYL_LATS; j++)
      for (i = 0; i <= CYL_LONGS; i++)
      {
         cylVertices[k].coords.x = cos((2*(float)i/CYL_LONGS) * M_PI );
         cylVertices[k].coords.y = sin((2*(float)i/CYL_LONGS) * M_PI );
         cylVertices[k].coords.z = (float)j/CYL_LATS;
		 cylVertices[k].coords.w = 1.0;
		 cylVertices[k].normal.x = cos( (2*(float)i/CYL_LONGS) * M_PI );
         cylVertices[k].normal.y = sin( (2*(float)i/CYL_LONGS) * M_PI );
		 cylVertices[k].normal.z = 0.0;
		 cylVertices[k].texCoords.x = 10*(float)i/CYL_LONGS;
		 cylVertices[k].texCoords.y = 20*(float)j/CYL_LATS;
		 k++;
	  }
}

// Fill the array of index arrays.
void fillCylIndices(unsigned int cylIndices[CYL_LATS][2*(CYL_LONGS+1)])
{
   int i, j;
   for(j = 0; j < CYL_LATS; j++)
   {
      for (i = 0; i <= CYL_LONGS; i++)
      {
	     cylIndices[j][2*i] = (j+1)*(CYL_LONGS + 1) + i;
	     cylIndices[j][2*i+1] = j*(CYL_LONGS + 1) + i;
      }
   }
}

// Fill the array of counts.
void fillCylCounts(int cylCounts[CYL_LATS])
{
   int j;
   for(j = 0; j < CYL_LATS; j++) cylCounts[j] = 2*(CYL_LONGS + 1);
}

// Fill the array of buffer offsets.
void fillCylOffsets(void* cylOffsets[CYL_LATS])
{
   int j;
   for(j = 0; j < CYL_LATS; j++) cylOffsets[j] = (GLvoid*)(2*(CYL_LONGS+1)*j*sizeof(unsigned int));
}

// Initialize the cylinder.
void fillCylinder(Vertex cylVertices[(CYL_LONGS + 1) * (CYL_LATS + 1)],
	         unsigned int cylIndices[CYL_LATS][2*(CYL_LONGS+1)],
			 int cylCounts[CYL_LATS],
			 void* cylOffsets[CYL_LATS])
{
   fillCylVertexArray(cylVertices);
   fillCylIndices(cylIndices);
   fillCylCounts(cylCounts);
   fillCylOffsets(cylOffsets);
}
