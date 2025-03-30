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


// I tryed to reduce to the minimun the number of required assets, while still keeping a varied design

// CYLINDER
layout(location=0) in vec4 cylCoords;
layout(location=1) in vec3 cylNormal;
layout(location=2) in vec2 cylTexCoords;

// RECTANGLE
layout(location=3) in vec4 rectangleCoords;
layout(location=4) in vec3 rectangleNormal;
layout(location=5) in vec2 rectangleTexCoords;

// ISOSCELES TRIANGLE
layout(location=6) in vec4 triangleCoords;
layout(location=7) in vec3 triangleNormal;
layout(location=8) in vec2 triangleTexCoords;

// SQUARE
layout(location=9) in vec4 squareCoords;
layout(location=10) in vec3 squareNormal;
layout(location=11) in vec2 squareTexCoords;

// TWO RECTANGLES CONNECTED WITH A RIGHT ANGLE
layout(location=12) in vec4 doubleRectangleCoords;
layout(location=13) in vec3 doubleRectangleNormal;
layout(location=14) in vec2 doubleRectangleTexCoords;


uniform mat4 modelViewMat;
uniform mat4 projMat;
uniform mat3 normalMat;
uniform uint object;

out vec4 frontAmbDiffExport, frontSpecExport, backAmbDiffExport, backSpecExport;
out vec2 texCoordsExport;

struct Light
{
    vec4 ambCols;
    vec4 difCols;
    vec4 specCols;
    vec4 coords;
};
uniform Light light0;

uniform vec4 globAmb;


struct Material
{
    vec4 ambRefl;
    vec4 difRefl;
    vec4 specRefl;
    vec4 emitCols;
    float shininess;
};
uniform Material wall_mat;
uniform Material roof_mat;
uniform Material window_mat;
uniform Material base_mat;
uniform Material door_mat;

vec3 normal, lightDirection, eyeDirection, halfway;
vec4 frontEmit, frontGlobAmb, frontAmb, frontDif, frontSpec,
     backEmit, backGlobAmb, backAmb, backDif, backSpec;
vec4 coords;


void main(void)
{
	Material mat;
    // Here we select which object we are plotting
    if (object == TOWER) {//0
        coords = cylCoords;
        normal = cylNormal;
        texCoordsExport = cylTexCoords;
        mat=wall_mat;
    }
    if (object == RECTANGLE_WALL) {//1
        coords = rectangleCoords;
        normal = rectangleNormal;
        texCoordsExport = rectangleTexCoords;
        mat=wall_mat;
    }
    if (object == TRIANGLE_WALL) {//2
    	coords = triangleCoords;
        normal = triangleNormal;
        texCoordsExport = triangleTexCoords;
        mat=wall_mat;
    }
    if (object == DOUBLE_RECTANGLE_WALL) {//3
    	coords = doubleRectangleCoords;
        normal = doubleRectangleNormal;
        texCoordsExport = doubleRectangleTexCoords;
        mat=wall_mat;
    }
    if (object == TRIANGLE_ROOF) {//4
    	coords = triangleCoords;
        normal = triangleNormal;
        texCoordsExport = triangleTexCoords;
        mat=roof_mat;
    }
    if (object == SQUARE_DECORATION) {//5
    	coords = squareCoords;
        normal = squareNormal;
        texCoordsExport = squareTexCoords;
        mat=wall_mat;
    }
    if (object==SQUARE_WINDOW) {//6
    	coords = squareCoords;
        normal = squareNormal;
        texCoordsExport = squareTexCoords;
        mat=window_mat;
    }
    if (object == RECTANGLE_BASE) {//7
    	coords = rectangleCoords;
        normal = rectangleNormal;
        texCoordsExport = rectangleTexCoords;
        mat=base_mat;
    }
    if (object == RECTANGLE_DOOR) {//8
    	coords = squareCoords;
        normal = squareNormal;
        texCoordsExport = squareTexCoords;
        mat=door_mat;
    }
    if (object == SQUARE_LANDSCAPE) {//9
    	coords = squareCoords;
        normal = squareNormal;
        texCoordsExport = squareTexCoords;
        mat=base_mat;
    }
    if (object == SQUARE_ENTRANCE) {//10
    	coords = squareCoords;
        normal = squareNormal;
        texCoordsExport = squareTexCoords;
        mat=base_mat;
    }
    
    
    
    
    // object normal
    normal = normalize(normalMat * normal);
    // direction of the light
    lightDirection = normalize(vec3(light0.coords));
    // view direction
    eyeDirection = -1.0f * normalize(vec3(modelViewMat * coords));
    halfway = (length(lightDirection + eyeDirection) == 0.0f) ?
              vec3(0.0) : (lightDirection + eyeDirection)/
              length(lightDirection + eyeDirection);

    // PHONG Model - FRONT FACE
    // That is, emitted light and global, diffuse and specular
    // components.
    frontEmit = mat.emitCols;
    frontGlobAmb = globAmb * mat.ambRefl;
    frontAmb = light0.ambCols * mat.ambRefl;
    frontDif = max(dot(normal, lightDirection), 0.0f) *
        (light0.difCols * mat.difRefl);
    frontSpec = pow(max(dot(normal, halfway), 0.0f),
                    mat.shininess) * (light0.specCols * mat.specRefl);
    frontAmbDiffExport =  vec4(vec3(min(frontEmit + frontGlobAmb +
        frontAmb + frontDif, vec4(1.0))), 1.0);
    frontSpecExport =  vec4(vec3(min(frontSpec, vec4(1.0))), 1.0);

    // PHONG Model - BACK FACE
    // That is, emitted light and global, diffuse and specular
    // components.
    normal = -1.0f * normal;
    backEmit = mat.emitCols;
    backGlobAmb = globAmb * mat.ambRefl;
    backAmb = light0.ambCols * mat.ambRefl;
    backDif = max(dot(normal, lightDirection), 0.0f) *
        (light0.difCols * mat.difRefl);
    backSpec = pow(max(dot(normal, halfway), 0.0f),
        mat.shininess) * (light0.specCols * mat.specRefl);
    backAmbDiffExport =  vec4(vec3(min(backEmit + backGlobAmb + backAmb +
        backDif, vec4(1.0))), 1.0);
    backSpecExport =  vec4(vec3(min(backSpec, vec4(1.0))), 1.0);

    gl_Position = projMat * modelViewMat * coords;
}
