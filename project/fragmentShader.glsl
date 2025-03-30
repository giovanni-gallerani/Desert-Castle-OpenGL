#version 430 core

#define TOWER 0
#define RECTANGLE_WALL 1
#define TRIANGLE_WALL 2
#define DOUBLE_RECTANGLE_WALL 3
#define TRIANGLE_ROOF 4
#define SQUARE_DECORATION 5
#define SQUARE_WINDOW 6
#define RECTANGLE_BASE 7
#define RECTANGLE_DOOR 8
#define SQUARE_LANDSCAPE 9
#define SQUARE_ENTRANCE 10

// Material's properties
in vec4 frontAmbDiffExport, frontSpecExport, backAmbDiffExport, backSpecExport;
// texture coordinates
in vec2 texCoordsExport;


// samplers for the textures
uniform sampler2D wallTex; //0
uniform sampler2D roofTex; //1
uniform sampler2D windowTex; //2
uniform sampler2D star_decTex; //3
uniform sampler2D baseTex; //4
uniform sampler2D doorTex; //5 
uniform sampler2D landscapeTex; //6
uniform sampler2D entranceTex; //7

uniform uint object;

out vec4 colorsOut;

vec4 texColor;

void main(void)
{
    // check which object we are currently drawing
	//oggetti con la texture del muro
	if (object == TOWER) texColor = texture(wallTex, texCoordsExport);
    if (object == RECTANGLE_WALL) texColor = texture(wallTex, texCoordsExport);
	if (object == TRIANGLE_WALL) texColor = texture(wallTex, texCoordsExport);
	if (object == DOUBLE_RECTANGLE_WALL) texColor = texture(wallTex, texCoordsExport);
	//oggetti con la texture del tetto
	if (object == TRIANGLE_ROOF) texColor = texture(roofTex, texCoordsExport);
	//oggetti con la texture delle decorazioni
	if (object == SQUARE_DECORATION) texColor = texture(star_decTex, texCoordsExport);
	//oggetti con la texture della finestra
	if (object == SQUARE_WINDOW) texColor = texture(windowTex, texCoordsExport);
	if (object == RECTANGLE_BASE) texColor = texture(baseTex, texCoordsExport);	
	if (object == RECTANGLE_DOOR) texColor = texture(doorTex, texCoordsExport);
	if (object == SQUARE_LANDSCAPE) texColor = texture(landscapeTex, texCoordsExport);
	
	if (object == SQUARE_ENTRANCE) texColor = texture(entranceTex, texCoordsExport);
	
    // set color of the fragment
    colorsOut = gl_FrontFacing? (frontAmbDiffExport * texColor + frontSpecExport) :
                               (backAmbDiffExport * texColor + backSpecExport);
}
