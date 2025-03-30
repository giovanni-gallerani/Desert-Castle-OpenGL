/** 
* ----------------------------------------------------------------------------- 
* Computer Graphics - A.A. 2021-2022
*
* Teacher: Antonino Casile
* Students: GIOVANNI GALLERANI
* ----------------------------------------------------------------------------
* Computer Graphics Project
*
* This project presents the model of a castle with walls inspired by the aesthetics of Nintendo 64 video games
*
* The castle initially protrudes from the sand, holding down the space key an animation makes the castle emerge from under the sand
* First the walls emerge and when these are almost completely raised the castle inside them also begins to rise
* During this animation the scene rotates
*
* At the end of the animation it is possible to raise and lower the drawbridge at will by pressing 'u' (up) and 'd' (down)
*
* Pressing 'f' enables/disables the free rotation mode of the scene (disabled by default), this mode was created with the aim of observing how the model was created
* since the animation and the model are based on a perspective game, it is so to be understood as a sort of debugging mode
* In free rotation mode press 'x', 'X', 'y', 'Y', 'z', 'Z' to rotate the scene.

* Pressing 'r' resets the animation and disables the free rotation setting of the scene
* -----------------------------------------------------------------------------
*/


// standard includes
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cglm/cglm.h>
#include <cglm/types-struct.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "shader.h"
#include "cylinder.h"
#include "rectangle.h"
#include "triangle.h"
#include "square.h"

#include "light.h"
#include "material.h"
#include "readBMP.h"

//altezza iniziale (y) a cui sono posti i modelli di mura e castello
#define initial_wall_height -3.7
#define initial_castle_height -6.0

//angolo iniziale di rotazione del ponte levatoio
#define initial_gateAngle 0.0


//definizione stack LIFO
typedef struct stack_element{
    mat4 element;
    struct stack_element* next;
}stack_item;

typedef stack_item* stack;

//inserisce la matrice in testa alla stack
stack pushMatrix(mat4 matrice, stack s){
    stack t = NULL;
    t = (stack)malloc(sizeof(stack_item));

    //copio la matrice nella stack
    for(int i=0; i<4;i++){
        for(int j=0; j<4;j++){
            t->element[i][j]=matrice[i][j];
        }
    }
    t->next = s;
    return t;
}

//restituisce la matrice in testa alla stack e elmimina l'elemento in testa
stack popMatrix(stack s, mat4 matrice){
    
    if(s==NULL){
        printf("\nErrore, è stato effettuato un pop su una stack vuota, chiusura programma\n");
        exit(-1);        
    }
    
    stack aux;

    //copio il valore della stack nella matrice
    for(int i=0; i<4;i++){
        for(int j=0; j<4;j++){
            matrice[i][j] = s->element[i][j];
        }
    }
    aux=s;
    aux=aux->next;
    free(s);
    return aux;
}

//stampa la stack ricorsivamente, utile ai fini di debugging
void print_stack(stack s){

    if(s!=NULL){
        for(int i=0; i<4; i++){
            for(int j=0; j<4; j++){
                printf("%f ,", s->element[i][j]);
            }
            printf("\n");
        }
        printf("\n");
        print_stack(s->next);
    }
}


static enum object {CYLINDER, RECTANGLE, TRIANGLE, SQUARE, DOUBLE_RECTANGLE}; // VAO ids.
static enum buffer {CYL_VERTICES, CYL_INDICES, RECTANGLE_VERTICES, TRIANGLE_VERTICES, SQUARE_VERTICES, DOUBLE_RECTANGLE_VERTICES}; // VBO ids.

// Globals.
static int animazione_finita = 0; //va a 1 quando l'animazione è terminata

static int rotazione_libera = 0; //cambia tra 0 e 1 premendo f
static float Xangle = 0.0, Yangle = 0.0, Zangle = 0.0; // Angoli per ruotare la scena (attivi in modalità rotazione libera)

static float wall_height = initial_wall_height; // altezza della base delle mura (serve a farle comparire da sotto la sabbia)
static float castle_height = initial_castle_height; // altezza della base del castello (serve a farlo comparire da sotto la sabbia)
static float gateAngle = initial_gateAngle; // angolo di inclinazione del portone


// Light properties.
static const Light light0 =
{
    (vec4){0.0, 0.0, 0.0, 1.0},
    (vec4){1.0, 1.0, 1.0, 1.0},
    (vec4){1.0, 1.0, 1.0, 1.0},
    (vec4){10.0, -5.0, 10.0, 0.0}
};

// Global ambient.
static const vec4 globAmb = (vec4)
{
    0.7, 0.7, 0.7, 1.0
};

// Front and back material properties.
static const Material wall_mat =
{
    (vec4){0.3, 0.3, 0.3, 1.0}, //ambient
    (vec4){1.0, 1.0, 1.0, 1.0}, //diffuse
    (vec4){0.1, 0.1, 0.1, 1.0}, //specular
    (vec4){0.0, 0.0, 0.0, 1.0}, //emitted
    20.0f //shinyness
};

static const Material roof_mat =
{
    (vec4){0.6, 0.6, 0.6, 1.0},
    (vec4){1.0, 1.0, 1.0, 1.0},
    (vec4){1.0, 1.0, 1.0, 1.0},
    (vec4){0.0, 0.0, 0.0, 1.0},
	50.0f
};

static const Material window_mat =
{
    (vec4){1.0, 1.0, 1.0, 1.0},
    (vec4){1.0, 1.0, 1.0, 1.0},
    (vec4){1.0, 1.0, 1.0, 1.0},
    (vec4){0.0, 0.0, 0.0, 1.0},
    50.0f
};

static const Material base_mat = //material used for the base, background and image behind the door
{
    (vec4){1.0, 1.0, 1.0, 1.0},
    (vec4){0.0, 0.0, 0.0, 1.0},
    (vec4){0.0, 0.0, 0.0, 1.0},
    (vec4){0.0, 0.0, 0.0, 1.0},
    50.0f
};

static const Material door_mat =
{
    (vec4){1.0, 1.0, 1.0, 1.0},
    (vec4){1.0, 1.0, 1.0, 1.0},
    (vec4){0.5, 0.5, 0.5, 1.0},
    (vec4){0.0, 0.0, 0.0, 1.0},
    50.0f
};

//ASSET UTILIZZATI

// Cylinder data.
static Vertex cylVertices[(CYL_LONGS + 1) * (CYL_LATS + 1)];
static unsigned int cylIndices[CYL_LATS][2*(CYL_LONGS+1)];
static int cylCounts[CYL_LATS];
static void* cylOffsets[CYL_LATS];

// Rectangle data.
static Vertex rectangleVertices[RECTANGLE_VERTEX];

// Triangle data.
static Vertex triangleVertices[TRIANGLE_VERTEX];

// Square data.
static Vertex squareVertices[SQUARE_VERTEX];

// Double rectangle data.
static Vertex doubleRectangleVertices[DOUBLE_RECTANGLE_VERTEX];


// Matrices
static mat4 modelViewMat = GLM_MAT4_IDENTITY_INIT;
static mat4 projMat = GLM_MAT4_IDENTITY_INIT;
static mat3 normalMat = GLM_MAT3_IDENTITY_INIT;

static unsigned int
programId,
vertexShaderId,
fragmentShaderId,
modelViewMatLoc,
normalMatLoc,
projMatLoc,
wallTexLoc,
roofTexLoc,
windowTexLoc,
star_decTexLoc,
baseTexLoc,
doorTexLoc,
landscapeTexLoc,
entranceTexLoc,
objectLoc,
buffer[6],
vao[5],
texture[8],
width,
height;

static BitMapFile *image[8]; // Local storage for bmp image data.

// Initialization routine
void setup(void)
{
    GLenum glErr;

      // ... it does not hurt to check that everything went OK
    if ((glErr = glGetError()) != 0)
    {
        printf("Errore = %d \n", glErr);
        exit(-1);
    }

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);

    // Create shader program executable.
    vertexShaderId = setShader("vertex", "vertexShader.glsl");
    fragmentShaderId = setShader("fragment", "fragmentShader.glsl");
    programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    glUseProgram(programId);

    // ... it does not hurt to check that everything went OK
    if ((glErr = glGetError()) != 0)
    {
        printf("Errore inizio = %d \n", glErr);
        exit(-1);
    }	
	
    // Initialize assets geometry.
    fillCylinder(cylVertices, cylIndices, cylCounts, cylOffsets);
    fillRectangleVertexArray(rectangleVertices);
    fillTriangleVertexArray(triangleVertices);
    fillSquareVertexArray(squareVertices);
    fillDoubleRectangleVertexArray(doubleRectangleVertices);
    
    // Create VAOs and VBOs...
    glGenVertexArrays(5, vao);
    glGenBuffers(6, buffer);

    // ...and associate data with vertex shader.
    glBindVertexArray(vao[CYLINDER]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[CYL_VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cylVertices), cylVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer[CYL_INDICES]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cylIndices), cylIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(cylVertices[0]), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(cylVertices[0]), (void*)sizeof(cylVertices[0].coords));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(cylVertices[0]),
                          (void*)(sizeof(cylVertices[0].coords)+sizeof(cylVertices[0].normal)));
    glEnableVertexAttribArray(2);

    // ...and associate data with vertex shader.
    glBindVertexArray(vao[RECTANGLE]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[RECTANGLE_VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(rectangleVertices[0]), 0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(rectangleVertices[0]), (void*)sizeof(rectangleVertices[0].coords));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(rectangleVertices[0]),
                          (void*)(sizeof(rectangleVertices[0].coords)+sizeof(rectangleVertices[0].normal)));
    glEnableVertexAttribArray(5);
    
    // ...and associate data with vertex shader.
    glBindVertexArray(vao[TRIANGLE]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[TRIANGLE_VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(triangleVertices[0]), 0);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(triangleVertices[0]), (void*)sizeof(triangleVertices[0].coords));
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(triangleVertices[0]),
                          (void*)(sizeof(triangleVertices[0].coords)+sizeof(triangleVertices[0].normal)));
    glEnableVertexAttribArray(8);
    
    // ...and associate data with vertex shader.
    glBindVertexArray(vao[SQUARE]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[SQUARE_VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(squareVertices[0]), 0);
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(10, 3, GL_FLOAT, GL_FALSE, sizeof(squareVertices[0]), (void*)sizeof(squareVertices[0].coords));
    glEnableVertexAttribArray(10);
    glVertexAttribPointer(11, 2, GL_FLOAT, GL_FALSE, sizeof(squareVertices[0]),
                          (void*)(sizeof(squareVertices[0].coords)+sizeof(squareVertices[0].normal)));
    glEnableVertexAttribArray(11);

	
	// ...and associate data with vertex shader.
    glBindVertexArray(vao[DOUBLE_RECTANGLE]);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[DOUBLE_RECTANGLE_VERTICES]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(doubleRectangleVertices), doubleRectangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(12, 4, GL_FLOAT, GL_FALSE, sizeof(doubleRectangleVertices[0]), 0);
    glEnableVertexAttribArray(12);
    glVertexAttribPointer(13, 3, GL_FLOAT, GL_FALSE, sizeof(doubleRectangleVertices[0]), (void*)sizeof(doubleRectangleVertices[0].coords));
    glEnableVertexAttribArray(13);
    glVertexAttribPointer(14, 2, GL_FLOAT, GL_FALSE, sizeof(doubleRectangleVertices[0]),
                          (void*)(sizeof(doubleRectangleVertices[0].coords)+sizeof(doubleRectangleVertices[0].normal)));
    glEnableVertexAttribArray(14);
    
    
    // Obtain modelview matrix, projection matrix, normal matrix and object uniform locations.
    modelViewMatLoc = glGetUniformLocation(programId, "modelViewMat");
    projMatLoc = glGetUniformLocation(programId, "projMat");
    normalMatLoc = glGetUniformLocation(programId, "normalMat");
    objectLoc = glGetUniformLocation(programId, "object");

    // Obtain light property uniform locations and set values.
    glUniform4fv(glGetUniformLocation(programId, "light0.ambCols"), 1, &light0.ambCols[0]);
    glUniform4fv(glGetUniformLocation(programId, "light0.difCols"), 1, &light0.difCols[0]);
    glUniform4fv(glGetUniformLocation(programId, "light0.specCols"), 1, &light0.specCols[0]);
    glUniform4fv(glGetUniformLocation(programId, "light0.coords"), 1, &light0.coords[0]);

    // Obtain global ambient uniform location and set value.
    glUniform4fv(glGetUniformLocation(programId, "globAmb"), 1, &globAmb[0]);

    // Obtain material property uniform locations and set values.
    glUniform4fv(glGetUniformLocation(programId, "wall_mat.ambRefl"), 1, &wall_mat.ambRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "wall_mat.difRefl"), 1, &wall_mat.difRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "wall_mat.specRefl"), 1, &wall_mat.specRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "wall_mat.emitCols"), 1, &wall_mat.emitCols[0]);
    glUniform1f(glGetUniformLocation(programId, "wall_mat.shininess"), wall_mat.shininess);

    // Obtain material property uniform locations and set values.
    glUniform4fv(glGetUniformLocation(programId, "roof_mat.ambRefl"), 1, &roof_mat.ambRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "roof_mat.difRefl"), 1, &roof_mat.difRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "roof_mat.specRefl"), 1, &roof_mat.specRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "roof_mat.emitCols"), 1, &roof_mat.emitCols[0]);
    glUniform1f(glGetUniformLocation(programId, "roof_mat.shininess"), roof_mat.shininess);
    
    // Obtain material property uniform locations and set values.
    glUniform4fv(glGetUniformLocation(programId, "window_mat.ambRefl"), 1, &window_mat.ambRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "window_mat.difRefl"), 1, &window_mat.difRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "window_mat.specRefl"), 1, &window_mat.specRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "window_mat.emitCols"), 1, &window_mat.emitCols[0]);
    glUniform1f(glGetUniformLocation(programId, "window_mat.shininess"), window_mat.shininess);

	// Obtain material property uniform locations and set values.
    glUniform4fv(glGetUniformLocation(programId, "base_mat.ambRefl"), 1, &base_mat.ambRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "base_mat.difRefl"), 1, &base_mat.difRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "base_mat.specRefl"), 1, &base_mat.specRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "base_mat.emitCols"), 1, &base_mat.emitCols[0]);
    glUniform1f(glGetUniformLocation(programId, "base_mat.shininess"), base_mat.shininess);

	// Obtain material property uniform locations and set values.
    glUniform4fv(glGetUniformLocation(programId, "door_mat.ambRefl"), 1, &door_mat.ambRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "door_mat.difRefl"), 1, &door_mat.difRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "door_mat.specRefl"), 1, &door_mat.specRefl[0]);
    glUniform4fv(glGetUniformLocation(programId, "door_mat.emitCols"), 1, &door_mat.emitCols[0]);
    glUniform1f(glGetUniformLocation(programId, "door_mat.shininess"), door_mat.shininess);
	
	
    // Load the images.
    image[0] = readBMP("./textures/wall.bmp");
    image[1] = readBMP("./textures/roof.bmp");
	image[2] = readBMP("./textures/window.bmp");
	image[3] = readBMP("./textures/star_dec.bmp");
	image[4] = readBMP("./textures/sand.bmp");
	image[5] = readBMP("./textures/door.bmp");
	image[6] = readBMP("./textures/landscape.bmp");
	image[7] = readBMP("./textures/entrance.bmp");
	
	
    // Create texture ids.
    glGenTextures(8, texture);

    // Bind wall image.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->sizeX, image[0]->sizeY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    wallTexLoc = glGetUniformLocation(programId, "wallTex");
    glUniform1i(wallTexLoc, 0);

    // Bind roof image.
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[1]->sizeX, image[1]->sizeY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[1]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    roofTexLoc = glGetUniformLocation(programId, "roofTex");
    glUniform1i(roofTexLoc, 1);
    
    // Bind window image.
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[2]->sizeX, image[2]->sizeY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[2]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    windowTexLoc = glGetUniformLocation(programId, "windowTex");
    glUniform1i(windowTexLoc, 2);
    
	// Bind decoration image.
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[3]->sizeX, image[3]->sizeY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[3]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    star_decTexLoc = glGetUniformLocation(programId, "star_decTex");
    glUniform1i(star_decTexLoc, 3);
    
    // Bind base image.
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[4]->sizeX, image[4]->sizeY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[4]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    baseTexLoc = glGetUniformLocation(programId, "baseTex");
    glUniform1i(baseTexLoc, 4);
    
    
    //Bind door image.
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, texture[5]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[5]->sizeX, image[5]->sizeY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[5]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    doorTexLoc = glGetUniformLocation(programId, "doorTex");
    glUniform1i(doorTexLoc, 5);
    
    //Bind landscape image.
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, texture[6]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[6]->sizeX, image[6]->sizeY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[6]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    landscapeTexLoc = glGetUniformLocation(programId, "landscapeTex");
    glUniform1i(landscapeTexLoc, 6);
    
    //Bind entrance image.
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, texture[7]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[7]->sizeX, image[7]->sizeY, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[7]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    entranceTexLoc = glGetUniformLocation(programId, "entranceTex");
    glUniform1i(entranceTexLoc, 7);
    
    
    
    
    // ... it does not hurt to check that everything went OK
    if ((glErr = glGetError()) != 0)
    {
        printf("Errore = %d \n", glErr);
        exit(-1);
    }

}

// Drawing routine.
void display(void)
{
    mat3 TMP;
    stack modelViewMatStack = NULL;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Calculate and update projection matrix.
    glm_perspective(14.0f, (float)width/(float)height, 1.0f, 50.0f, projMat);
    glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, (GLfloat *) projMat);
	

	// Calculate and update modelview matrix.
    glm_mat4_identity(modelViewMat);
	glm_lookat((vec3){3.5, 2.0, 8.0}, (vec3){3.5, 3.0, 0.0}, (vec3){0.0, 1.0, 0.0}, modelViewMat); 
    //ruoto la scena
	glm_rotate(modelViewMat, Xangle, (vec3){1.0, 0.0, 0.0});
	glm_rotate(modelViewMat, Yangle, (vec3){0.0, 1.0, 0.0});
	glm_rotate(modelViewMat, Zangle, (vec3){0.0, 0.0, 1.0});
    	
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	
	

	//inizio a disegnare gli oggetti nella scena
	/*Per prima cosa disegno la parte frontale delle mura, costituita da due torri (sinistra e destra)
	* proseguo poi posizionando i due muri (dritto e inclinato) che uniscono le torri,
	* le decorazioni sul muro, l'entrata e infine il ponte levatoio
	*/
	
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack); //faccio la push prima di disegnare la cinta muraria

	glm_translate(modelViewMat, (vec3){0.0, wall_height, 0.0}); 

	//TORRI AVANTI ---------------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

    //TORRE SINISTRA AVANTI

	//POSIZIONAMENTO
	glm_rotate(modelViewMat, -M_PI/2, (vec3){1.0, 0.0, 0.0});
	glm_scale(modelViewMat, (vec3){0.6, 0.6, 3.6}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
    glm_mat4_pick3(modelViewMat, TMP);
    glm_mat3_inv(TMP, normalMat);
    glm_mat3_transpose(normalMat);
    glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 0);
	glBindVertexArray(vao[CYLINDER]);
	glMultiDrawElements(GL_TRIANGLE_STRIP, cylCounts, GL_UNSIGNED_INT,
	(const void **)cylOffsets, CYL_LATS);
        	
    //TORRE DESTRA AVANTI
	
	//POSIZIONAMENTO
	//la rotazione sull'asse x è già fornita dalla precedente torre
	//traslo dividendo per 0.6 in modo da sfruttare lo scaling e la rotazione calcolati per la precedente torre
	glm_translate(modelViewMat, (vec3){7.0/0.6, 0.0, 0.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
    glm_mat4_pick3(modelViewMat, TMP);
    glm_mat3_inv(TMP, normalMat);
    glm_mat3_transpose(normalMat);
    glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
    glUniform1ui(objectLoc, 0);
    glBindVertexArray(vao[CYLINDER]);
    glMultiDrawElements(GL_TRIANGLE_STRIP, cylCounts, GL_UNSIGNED_INT,
    (const void **)cylOffsets, CYL_LATS);

	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);



	//FINESTRE TORRI AVANTI ----------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//FINESTRA TORRE SINISTRA

	//POSIZIONAMENTO x=0.0-0.1   y=2.7
	glm_translate(modelViewMat, (vec3){-0.1, 2.7, 0.6}); //metto la finestra appena sopra al muro delle torri
	glm_scale(modelViewMat, (vec3){0.2, 0.4, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 6);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);


	//FINESTRA TORRE DESTRA

	//POSIZIONAMENTO
	//traslo dividendo per 0.6 in modo da sfruttare lo scaling calcolato per la precedente finestra
	glm_translate(modelViewMat, (vec3){7.0/0.2, 0.0, 0.0});
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 6);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);

	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);



	//MURO INCLINATO CENTRALE ----------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){0.0, 0.0, 0.5});
	glm_rotate(modelViewMat, -M_PI/6, (vec3){1.0, 0.0, 0.0});
	glm_scale(modelViewMat, (vec3){7.0, 1.0, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 1);
	glBindVertexArray(vao[RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);

	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);



	//MURO CENTRALE ------------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

    //POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){0.6, 0.7, 0.0});
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);//lo uso dopo per le decorazioni
	glm_scale(modelViewMat, (vec3){5.8, 2.0, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 1);
	glBindVertexArray(vao[RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);

	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);
	
		
		
	//DECORAZIONI MURO CENTRALE -------------------------------------------------------------------------------------

	//metto la decorazione con base 0.1 prima della fine del muro (rispetto alla base del muro dritto è alzato di 1.9)
	//per dare l'effetto della tridimensionalità metto la decorazione leggermente staccata dalla parete 0.05
	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){0.0, 1.9, 0.05}); 
    glm_scale(modelViewMat, (vec3){1.16, 0.1, 1.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	    	
	//disegno altri 4 oggetti decorativi uguali al primo uno dopo l'altro
	//ogni volta mi sposto a destra di 1 che nel sistema di riferimento originario equivale alla lunghezza del singolo pezzo di decorazione
	for(int j=0; j<4; j++){
		glm_translate(modelViewMat, (vec3){1.0, 0.0 ,0.0});
		glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    	// Calculate and update normal matrix.
		glm_mat4_pick3(modelViewMat, TMP);
		glm_mat3_inv(TMP, normalMat);
		glm_mat3_transpose(normalMat);
		glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
		// DISEGNO
		glUniform1ui(objectLoc, 5);
		glBindVertexArray(vao[SQUARE]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	}

	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);



	//MURO TRIANGOLARE ---------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){3, 2.7, 0.0});
	glm_scale(modelViewMat, (vec3){0.5, 0.5, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 2);
	glBindVertexArray(vao[TRIANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);

	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);



	//DECORAZIONI MURO TRIANGOLARE ---------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//questa decorazione la posiziono leggermente più esterna rispetto alla precedente, 
    //in questo modo evito che le textures non si sovrappongano 
    //incremento quindi la z di 0.01

	//DECORAZIONE SINISTRA

	//POSIZIONAMENTO
    glm_translate(modelViewMat, (vec3){3.0, 2.6, 0.06}); //z incrementata di 0.01 rispetto al caso precedente
	glm_rotate(modelViewMat, M_PI/4, (vec3){0.0, 0.0, 1.0});
    glm_scale(modelViewMat, (vec3){0.8, 0.1, 1.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	    
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//DECORAZIONE DESTRA
	
	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){3.5, 3.2, 0.07}); //z incrementata di 0.01 rispetto al caso precedente 		
	glm_rotate(modelViewMat, -M_PI/4, (vec3){0.0, 0.0, 1.0});
	glm_scale(modelViewMat, (vec3){0.8, 0.1, 1.0});
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	
	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);



	//ENTRATA CASTELLO (l'immagine che sta dietro al ponte levatoio)-------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

    //POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){3.0, 0.85, 0.01});
	glm_scale(modelViewMat, (vec3){1.0, 1.7, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 10);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);

	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);



	//PORTONE-------------------------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

    //POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){3.0, 0.85, 0.05});
	glm_rotate(modelViewMat, gateAngle, (vec3){1.0, 0.0, 0.0});	//l'inclinazione del portone è data dalla pressione dei tasti u (up) e d (down)
	glm_scale(modelViewMat, (vec3){1.0, 1.7, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 8);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);

	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);



	//TORRE SINISTRA DIETRO ---------------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//POSIZIONAMENTO
    glm_rotate(modelViewMat, -M_PI/6, (vec3){0.0, 1.0, 0.0});
    glm_translate(modelViewMat, (vec3){-5.0, 0.0, 0.0});
	glm_rotate(modelViewMat, -M_PI/2, (vec3){1.0, 0.0, 0.0});
	glm_scale(modelViewMat, (vec3){0.6, 0.6, 3.6}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
    glm_mat4_pick3(modelViewMat, TMP);
    glm_mat3_inv(TMP, normalMat);
    glm_mat3_transpose(normalMat);
    glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
    //DISEGNO
    glUniform1ui(objectLoc, 0);
    glBindVertexArray(vao[CYLINDER]);
    glMultiDrawElements(GL_TRIANGLE_STRIP, cylCounts, GL_UNSIGNED_INT,
    (const void **)cylOffsets, CYL_LATS);

	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);



	//MURO INCLINATO SINISTRA ------------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//POSIZIONAMENTO
	glm_rotate(modelViewMat, -M_PI/6, (vec3){0.0, 1.0, 0.0});
	glm_translate(modelViewMat, (vec3){-5.0, 0.0, 0.5});
	glm_rotate(modelViewMat, -M_PI/6, (vec3){1.0, 0.0, 0.0});
	glm_scale(modelViewMat, (vec3){5.0, 1.0, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 1);
	glBindVertexArray(vao[RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);
    
	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);



	//MURO SINISTRA ------------------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//POSIZIONAMENTO
    glm_rotate(modelViewMat, -M_PI/6, (vec3){0.0, 1.0, 0.0});
	glm_translate(modelViewMat, (vec3){-4.4, 0.7, 0.0});
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);//utilizzo questa rotazione e traslazione per le decorazioni
	glm_scale(modelViewMat, (vec3){3.9, 2.0, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 1);
	glBindVertexArray(vao[RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);			    

	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);



	//DECORAZIONI MURO SINISTRA -------------------------------------------------------------------------------------

	//metto la decorazione con base 0.1 prima della fine del muro
	//per dare l'effetto della tridimensionalità metto la decorazione leggermente staccata dalla parete 0.05
	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){0.0, 1.9, 0.05}); 
    glm_scale(modelViewMat, (vec3){1.3, 0.1, 1.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	    	
	//disegno altri 4 oggetti decorativi uguali al primo uno dopo l'altro
	//ogni volta mi sposto a destra di 1 che nel sistema di riferimento originario equivale alla lunghezza del singolo pezzo di decorazione
	for(int j=0; j<2; j++){
		glm_translate(modelViewMat, (vec3){1.0, 0.0 ,0.0});
		glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    	// Calculate and update normal matrix.
		glm_mat4_pick3(modelViewMat, TMP);
		glm_mat3_inv(TMP, normalMat);
		glm_mat3_transpose(normalMat);
		glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
		// DISEGNO
		glUniform1ui(objectLoc, 5);
		glBindVertexArray(vao[SQUARE]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	}

	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);



	//TORRE DESTRA DIETRO --------------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//POSIZIONAMENTO
    glm_translate(modelViewMat, (vec3){7.0, 0.0, 0.0});
    glm_rotate(modelViewMat, M_PI/6, (vec3){0.0, 1.0, 0.0});
	glm_translate(modelViewMat, (vec3){5.0, 0.0, 0.0});
	glm_rotate(modelViewMat, -M_PI/2, (vec3){1.0, 0.0, 0.0});
	glm_scale(modelViewMat, (vec3){0.6, 0.6, 3.6}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
    glm_mat4_pick3(modelViewMat, TMP);
    glm_mat3_inv(TMP, normalMat);
    glm_mat3_transpose(normalMat);
    glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
    //DISEGNO
    glUniform1ui(objectLoc, 0);
    glBindVertexArray(vao[CYLINDER]);
    glMultiDrawElements(GL_TRIANGLE_STRIP, cylCounts, GL_UNSIGNED_INT,
    (const void **)cylOffsets, CYL_LATS);

	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);
		


	//MURO INCLINATO DESTRA -------------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){7.0, 0.0, 0.0});
	glm_rotate(modelViewMat, M_PI/6, (vec3){0.0, 1.0, 0.0});
	glm_translate(modelViewMat, (vec3){0.0, 0.0, 0.5});
	glm_rotate(modelViewMat, -M_PI/6, (vec3){1.0, 0.0, 0.0});
	glm_scale(modelViewMat, (vec3){5.0, 1.0, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 1);
	glBindVertexArray(vao[RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);
	
	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);
	    


	//MURO DESTRA ----------------------------------------------------------------------------------------------------------- 
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

    //POSIZIONAMENTO
    glm_translate(modelViewMat, (vec3){7.0, 0.0, 0.0});
    glm_rotate(modelViewMat, M_PI/6, (vec3){0.0, 1.0, 0.0});
	glm_translate(modelViewMat, (vec3){0.5, 0.7, 0.0});
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);//utilizzo questa rotazione e traslazione per le decorazioni
	glm_scale(modelViewMat, (vec3){3.9, 2.0, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 1);
	glBindVertexArray(vao[RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);
		
	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);
	
	

	//DECORAZIONI MURO SINISTRA -------------------------------------------------------------------------------------

	//metto la decorazione con base 0.1 prima della fine del muro
	//per dare l'effetto della tridimensionalità metto la decorazione leggermente staccata dalla parete 0.05
	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){0.0, 1.9, 0.05}); 
    glm_scale(modelViewMat, (vec3){1.3, 0.1, 1.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	    	
	//disegno altri 4 oggetti decorativi uguali al primo uno dopo l'altro
	//ogni volta mi sposto a destra di 1 che nel sistema di riferimento originario equivale alla lunghezza del singolo pezzo di decorazione
	for(int j=0; j<2; j++){
		glm_translate(modelViewMat, (vec3){1.0, 0.0 ,0.0});
		glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    	// Calculate and update normal matrix.
		glm_mat4_pick3(modelViewMat, TMP);
		glm_mat3_inv(TMP, normalMat);
		glm_mat3_transpose(normalMat);
		glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
		// DISEGNO
		glUniform1ui(objectLoc, 5);
		glBindVertexArray(vao[SQUARE]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	}

	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);

    
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat); //ho finito di disegnare la cinta muraria


	





	/*Ho terminato il disegno della cinta muraria, devo ora disegnare nella scena il castello
	* Il castello è costituito da edifici, con eventuali finestre, decorazioni e guglie o tetti
	*/







	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack); //faccio la push prima di disegnare il castello
	glm_translate(modelViewMat, (vec3){0.0, castle_height, 0.0});
	
	//EDIFICIO CENTRALE AVANTI *********************************************************************************************************

	//EDIFICIO CENTRALE AVANTI - MURA ----------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

    //largo 2 e alto 1.5 e va in profondità di 1.0
	//il suo angolo sinistro dista 1.4 dall'origine sull'asse x -> x = 1.4
    //la sua base inizia con la fine delle mura (y = 2.7)
	//dista dalle mura 1.5 -> z = -1.5
    //POSIZIONAMENTO
    glm_translate(modelViewMat, (vec3){1.4, 2.7, -1.5});  
    glm_scale(modelViewMat, (vec3){2.0, 1.5, 1.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 3);
	glBindVertexArray(vao[DOUBLE_RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);
    glDrawArrays(GL_TRIANGLE_STRIP, RECTANGLE_VERTEX, RECTANGLE_VERTEX);   	
    	
	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);


	//EDIFICIO CENTRALE AVANTI - GUGLIA -----------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//PRIMA PARTE GUGLIA (davanti)

    //POSIZIONAMENTO
    glm_translate(modelViewMat, (vec3){2.1, 4.2, -1.8});
    glm_rotate(modelViewMat, -M_PI*0.072, (vec3){1.0, 0.0, 0.0});
    glm_scale(modelViewMat, (vec3){0.3, 1.334, 1.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 4);
	glBindVertexArray(vao[TRIANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, TRIANGLE_VERTEX);

    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//SECONDA PARTE GUGLIA (destra)

    //POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){2.7, 4.2, -1.8});
	glm_rotate(modelViewMat, M_PI/2, (vec3){0.0, 1.0, 0.0});
	glm_rotate(modelViewMat, -M_PI*0.072, (vec3){1.0, 0.0, 0.0});
	glm_scale(modelViewMat, (vec3){0.3, 1.334, 1.0});
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 4);
	glBindVertexArray(vao[TRIANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, TRIANGLE_VERTEX);

	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);


	//EDIFICIO CENTRALE AVANTI - FINESTRE ------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//POSIZIONAMENTO x=1.4+0.7   y=2.7+0.8
	glm_translate(modelViewMat, (vec3){2.1, 3.5, -1.49}); //metto la finestra appena sopra al muro distanziando il quadrato su cui è dì 0.01 dalla parete
	glm_scale(modelViewMat, (vec3){0.2, 0.3, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 6);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	    	
	//disegno una seconda finestra affiancata alla prima

	glm_translate(modelViewMat, (vec3){1.5, 0.0, 0.0});  
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 6);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	    	
	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);





	//EDIFICIO SINISTRA INDIETRO ***************************************************************************************************

    //EDIFICIO SINISTRA INDIETRO - MURA -------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

    //largo 1.1 e alto 1.7 e va in profondità di 0.5
	//il suo angolo sinistro dista 0.3 dall'origine sull'asse x -> x = 0.3
    //la sua base inizia poco sopra la fine delle mura (y = 2.8)
	//dista dalle mura 2.0 -> z = -2.0
    //POSIZIONAMENTO
    glm_translate(modelViewMat, (vec3){0.3, 2.8, -2.0});
    glm_scale(modelViewMat, (vec3){1.1, 1.7, 0.5});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 3);
	glBindVertexArray(vao[DOUBLE_RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);
    glDrawArrays(GL_TRIANGLE_STRIP, RECTANGLE_VERTEX, RECTANGLE_VERTEX);		

	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);


	//EDIFICIO SINISTRA INDIETRO - GUGLIA -----------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//PRIMA PARTE GUGLIA (davanti)

	//POSIZIONAMENTO
    glm_translate(modelViewMat, (vec3){0.3, 4.5, -2.0});
	glm_rotate(modelViewMat, -M_PI*0.265, (vec3){1.0, 0.0, 0.0});
	glm_scale(modelViewMat, (vec3){0.55, 0.743, 1.0});
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 4);
	glBindVertexArray(vao[TRIANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, TRIANGLE_VERTEX);
    	
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//SECONDA PARTE GUGLIA (destra)

    //POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){1.4, 4.5, -2.0});
    glm_rotate(modelViewMat, M_PI/2, (vec3){0.0, 1.0, 0.0});
    glm_rotate(modelViewMat, -M_PI*0.265, (vec3){1.0, 0.0, 0.0});
    glm_scale(modelViewMat, (vec3){0.55, 0.743, 1.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 4);
	glBindVertexArray(vao[TRIANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, TRIANGLE_VERTEX);
		
	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);


	//EDIFICIO SINISTRA INDIETRO - FINESTRA ------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//POSIZIONAMENTO x=0.3+0.3   y=2.8+1.2
	glm_translate(modelViewMat, (vec3){0.6, 4.0, -1.99}); //metto la finestra appena sopra al muro distanziando il quadrato su cui è dì 0.01 dalla parete
	glm_scale(modelViewMat, (vec3){0.3, 0.3, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 6);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	
	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);





	//EDIFICIO DESTRA AVANTI *****************************************************************************************************

	//EDIFICIO DESTRA AVANTI - MURA -------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

    //largo 1.5 e alto 1.5 e va in profondità di 1.5
	//il suo angolo sinistro dista 4.4 dall'origine sull'asse x -> x = 4.4
    //la sua base inizia poco sopra la fine delle mura (y = 2.8)
	//dista dalle mura 1.9 -> z = -1.9
    //POSIZIONAMENTO	
    glm_translate(modelViewMat, (vec3){4.4, 2.8, -1.9});
    glm_scale(modelViewMat, (vec3){1.5, 1.5, 1.5});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 3);
	glBindVertexArray(vao[DOUBLE_RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);
    glDrawArrays(GL_TRIANGLE_STRIP, RECTANGLE_VERTEX, RECTANGLE_VERTEX);

	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);


	//EDIFICIO DESTRA AVANTI - GUGLIA ---------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//PRIMA PARTE GUGLIA (davanti) 
	
	//POSIZIONAMENTO
    glm_translate(modelViewMat, (vec3){4.9, 4.3, -2.0});
    glm_rotate(modelViewMat, -M_PI*0.082, (vec3){1.0, 0.0, 0.0});
    glm_scale(modelViewMat, (vec3){0.45, 1.758, 1.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 4);
	glBindVertexArray(vao[TRIANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, TRIANGLE_VERTEX);

    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//SECONDA PARTE GUGLIA (destra)

	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){5.8, 4.3, -2.0});
    glm_rotate(modelViewMat, M_PI/2, (vec3){0.0, 1.0, 0.0});
    glm_rotate(modelViewMat, -M_PI*0.082, (vec3){1.0, 0.0, 0.0});
    glm_scale(modelViewMat, (vec3){0.45, 1.758, 1.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 4);
	glBindVertexArray(vao[TRIANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, TRIANGLE_VERTEX);
	
	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);


	//EDIFICIO DESTRA AVANTI - DECORAZIONE ---------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//PRIMO LATO (davanti)
		
	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){4.38, 4.2, -1.85});//metto la decorazione con base y=0.1 prima della fine superiore del muro dell'edificio
    //nella glm_translate la decorazione è stata piazzata in 4.38 anzichè 4.4 in modo da sporgere leggermente dall'angolo sinistro dando l'impressione della tridimensionalità
	//la z della decorazione è lontana 0.05 dalle mura in modo che sembri applicata al muro come rilievo ma non si veda spazio vuoto sotto di essa
	glm_scale(modelViewMat, (vec3){1.54, 0.1, 1.0});//scalo di 1.54 lungo x in modo da poter poi piazzare correttamante il secondo pezzo della decorazione
    //evitando che finisca dentro al muro laterale e allo stesso modo avere una lieve sporgenza dal lato sinistro (0.02)
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	    	
	//SECONDO LATO (destra)

	//POSIZIONAMENTO
	//per porre il secondo lato prima mi sposto alla fine della decorazione appena tracciata
	glm_translate(modelViewMat, (vec3){1.0, 0.0, 0.0});
	//e poi ruoto di 90 gradi il sistema di riferimento
	glm_rotate(modelViewMat, M_PI/2, (vec3){0.0, 1.0, 0.0});
	glm_scale(modelViewMat, (vec3){1.57, 1.0, 1.0}); //0.05+1.5+0.02
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	
	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);


	//EDIFICIO DESTRA AVANTI - FINESTRA -----------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//POSIZIONAMENTO x=4.4+0.625.   y=2.8+0.8
	glm_translate(modelViewMat, (vec3){5.025, 3.6, -1.89}); //metto la finestra appena sopra al muro distanziando il quadrato su cui è dì 0.01 dalla parete
	glm_scale(modelViewMat, (vec3){0.25, 0.28, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 6);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	
	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);





	//EDIFICIO DESTRA INDIETRO ********************************************************************************************************

	//EDIFICIO DESTRA INDIETRO - MURA -----------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

    //largo 1.5 e alto 1.0 e il suo angolo sinistro dista 5.2 dall'origine sull'asse x -> x = 5.2
    //la sua base inizia poco sopra la fine delle mura (y = 3.0) e dista dalle mura 5.4 -> z = -5.4
    //va in profondità di 1.5
    //POSIZIONAMENTO
    glm_translate(modelViewMat, (vec3){5.2, 3.0, -5.4});
	glm_scale(modelViewMat, (vec3){1.5, 1.0, 1.5});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 3);
	glBindVertexArray(vao[DOUBLE_RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);
    glDrawArrays(GL_TRIANGLE_STRIP, RECTANGLE_VERTEX, RECTANGLE_VERTEX);
    
	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);


	//EDIFICIO DESTRA INDIETRO - DECORAZIONE -------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);
			
	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){5.2, 3.9, -5.35});//metto la decorazione con base 0.1 prima della fine superiore del muro dell'edificio
	//la z della decorazione è lontana 0.05 dalle mura in modo che sembri applicata al muro come rilievo ma non si veda spazio vuoto sotto di essa
	glm_scale(modelViewMat, (vec3){1.52, 0.1, 1.0});//scalo di 1.52 lungo y in modo che sporga leggermente dal lato destro dell'edificio dando l'imppressione di tridimensionalità
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
 	//non disegno la decorazione sul secondo lato dato che per via della prospettiva non si vedrebbe
	
	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);


	//EDIFICIO DESTRA INDIETRO - FINESTRE ----------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//POSIZIONAMENTO x=5.2+0.6   y=3.0+0.25
	glm_translate(modelViewMat, (vec3){5.8, 3.25, -5.39}); //metto la finestra appena sopra al muro distanziando il quadrato su cui è dì 0.01 dalla parete
	glm_scale(modelViewMat, (vec3){0.2, 0.3, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 6);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	    	
	//disegno una seconda finestra affiancata alla prima

	glm_translate(modelViewMat, (vec3){1.5, 0.0, 0.0});  
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 6);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);

	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);





	//EDIFICIO CENTRALE INDIETRO ****************************************************************************************************


	//EDIFICIO CENTRALE INDIETRO - MURA -------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);
    	
    //QUINTA PARTE CASTELLO (EDIFICIO CENTRALE GRANDE INDIETRO)
    //largo 3.6 e alto 2.1 e va in profondità di 2.0
	//il suo angolo sinistro dista 1.1 dall'origine sull'asse x -> x = 1.1
    //la sua base inizia poco sopra la fine delle mura (y = 2.9)
	//dista dalle mura 2.5 -> z = -2.5
    //POSIZIONAMENTO
    glm_translate(modelViewMat, (vec3){1.1, 2.9, -2.5});
    glm_scale(modelViewMat, (vec3){3.6, 2.1, 2.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 3);
	glBindVertexArray(vao[DOUBLE_RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);
    glDrawArrays(GL_TRIANGLE_STRIP, RECTANGLE_VERTEX, RECTANGLE_VERTEX);
    
	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);


	//EDIFICIO CENTRALE INDIETRO - GUGLIA -------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);
    
	//PRIMA PARTE GUGLIA (davanti)

	//POSIZIONAMENTO
    glm_translate(modelViewMat, (vec3){1.2, 5.0, -2.6});
    glm_rotate(modelViewMat, -M_PI*0.082, (vec3){1.0, 0.0, 0.0});
    glm_scale(modelViewMat, (vec3){0.45, 1.758, 1.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 4);
	glBindVertexArray(vao[TRIANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, TRIANGLE_VERTEX);
    
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//SECONDA PARTE GUGLIA (destra) 

	//POSIZIONAMENTO
    glm_translate(modelViewMat, (vec3){2.1, 5.0, -2.6});
    glm_rotate(modelViewMat, M_PI/2, (vec3){0.0, 1.0, 0.0});
    glm_rotate(modelViewMat, -M_PI*0.082, (vec3){1.0, 0.0, 0.0});
    glm_scale(modelViewMat, (vec3){0.45, 1.758, 1.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 4);
	glBindVertexArray(vao[TRIANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, TRIANGLE_VERTEX);

	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);


	//EDIFICIO CENTRALE INDIETRO - DECORAZIONE -------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//PRIMO LATO (davanti)

	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){1.08, 4.9, -2.45});//metto la decorazione con base y=0.1 prima della fine superiore del muro dell'edificio
	//nella glm_translate la decorazione è stata piazzata in modo da sporgere leggermente dall'angolo sinistro dando l'impressione della tridimensionalità
	//la z della decorazione è lontana 0.05 dalle mura in modo che sembri applicata al muro come rilievo ma non si veda spazio vuoto sotto di essa
	glm_scale(modelViewMat, (vec3){3.64, 0.1, 1.0});//scalo di 3.64 (0.02 in più a destra e sinistra) lungo x in modo da poter poi piazzare correttamante il secondo pezzo della decorazione
	//evitando che finisca dentro al muro laterale
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	    	
	//SECONDO LATO (sinistra)

	//per porre il secondo lato prima mi sposto alla fine della decorazione appena tracciata
	glm_translate(modelViewMat, (vec3){1.0, 0.0, 0.0});
	//e poi ruoto di 90 gradi il sistema di riferimento
	glm_rotate(modelViewMat, M_PI/2, (vec3){0.0, 1.0, 0.0});
	glm_scale(modelViewMat, (vec3){2.07, 1.0, 1.0}); //0.05+2.0+0.02 per dare l'effetto del rilievo	    	
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	
	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);





	//TORRE CENTRALE ***************************************************************************************************************

	//TORRE CENTRALE - MURA --------------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

    //la sesta torre interna è larga 0.9 e alta 2.0 e va in profondità di 0.9 (come la sua larghezza)
	//il suo angolo sinistro dista 3.1 dall'origine sull'asse x -> x = 3.1
    //la sua base inizia sopra la fine della quinta parte del castello (y = 5.0)
	//dista dalle mura 2.6 -> z = -2.6

	//la torre centrale viene disegnata in due parti, ciascuno alta 1.0

	//PARTE BASSA

    //POSIZIONAMENTO 1    
	glm_translate(modelViewMat, (vec3){3.1, 5.0, -2.6});
    glm_scale(modelViewMat, (vec3){0.9, 1.0, 0.9});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	 glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO 1
	glUniform1ui(objectLoc, 3);
	glBindVertexArray(vao[DOUBLE_RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);
	glDrawArrays(GL_TRIANGLE_STRIP, RECTANGLE_VERTEX, RECTANGLE_VERTEX);
    
	//PARTE ALTA

    //POSIZIONAMENTO 2	
    glm_translate(modelViewMat, (vec3){0.0, 1.0, 0.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO 2
	glUniform1ui(objectLoc, 3);
	glBindVertexArray(vao[DOUBLE_RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);
    glDrawArrays(GL_TRIANGLE_STRIP, RECTANGLE_VERTEX, RECTANGLE_VERTEX);
    		
	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);

    	
    //TORRE CENTRALE - GUGLIA ------------------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//PRIMA PARTE GUGLIA (davanti)

	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){3.1, 7.0, -2.6});
    glm_rotate(modelViewMat, -M_PI*0.082, (vec3){1.0, 0.0, 0.0});
    glm_scale(modelViewMat, (vec3){0.45, 1.758, 1.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
    //DISEGNO
	glUniform1ui(objectLoc, 4);
	glBindVertexArray(vao[TRIANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, TRIANGLE_VERTEX);

	
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);	
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);
    	
	//SECONDA PARTE GUGLIA (destra)

	//POSIZIONAMENTO
    glm_translate(modelViewMat, (vec3){4.0, 7.0, -2.6});
    glm_rotate(modelViewMat, M_PI/2, (vec3){0.0, 1.0, 0.0});
    glm_rotate(modelViewMat, -M_PI*0.082, (vec3){1.0, 0.0, 0.0});
    glm_scale(modelViewMat, (vec3){0.45, 1.758, 1.0});
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 4);
	glBindVertexArray(vao[TRIANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, TRIANGLE_VERTEX);
    	
	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);

	
	//TORRE CENTRALE - DECORAZIONI ---------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	//DECORAZIONE BASSA
	//0.65 dalla base della torre y=(5.0 + 0.65)

	//PRIMO LATO (davanti)

	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){3.08, 5.65, -2.58});//metto la decorazione con base y=0.1 prima della fine superiore del muro dell'edificio
	//nella glm_translate la decorazione è stata piazzata in modo da sporgere leggermente dall'angolo sinistro dando l'impressione della tridimensionalità 3.08=3.1-0.02
	//la z della decorazione è lontana 0.02 dalle mura della torre in modo che sembri applicata al muro come rilievo ma non si veda spazio vuoto sotto di essa
	glm_scale(modelViewMat, (vec3){0.94, 0.1, 1.0});//scalo di 0.94 (0.02 in più a destra e sinistra) lungo x in modo da poter poi piazzare correttamante il secondo pezzo della decorazione
	//evitando che finisca dentro al muro laterale
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	
	//SECONDO LATO (destra)

	//per porre il secondo lato prima mi sposto alla fine della decorazione appena tracciata
	glm_translate(modelViewMat, (vec3){1.0, 0.0, 0.0});
	//e poi ruoto di 90 gradi il sistema di riferimento
	glm_rotate(modelViewMat, M_PI/2, (vec3){0.0, 1.0, 0.0});
	glm_scale(modelViewMat, (vec3){0.94, 1.0, 1.0}); //0.02+9.0+0.02 per dare l'effetto del rilievo	    	
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
    // Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);
	
	//DECORAZIONE BASSA
	//1.3 dalla base della torre y=(5.0 + 1.3)

	//PRIMO LATO (davanti)

	//POSIZIONAMENTO
	glm_translate(modelViewMat, (vec3){3.08, 6.3, -2.58});//metto la decorazione con base y=0.1 prima della fine superiore del muro dell'edificio
	//nella glm_translate la decorazione è stata piazzata in modo da sporgere leggermente dall'angolo sinistro dando l'impressione della tridimensionalità 3.08=3.1-0.02
	//la z della decorazione è lontana 0.02 dalle mura della torre in modo che sembri applicata al muro come rilievo ma non si veda spazio vuoto sotto di essa
	glm_scale(modelViewMat, (vec3){0.94, 0.1, 1.0});//scalo di 0.94 (0.02 in più a destra e sinistra) lungo x in modo da poter poi piazzare correttamante il secondo pezzo della decorazione
	//evitando che finisca dentro al muro laterale
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	    	
	//SECONDO LATO (destra)

	//POSIZIONAMENTO
	//per porre il secondo lato prima mi sposto alla fine della decorazione appena tracciata
	glm_translate(modelViewMat, (vec3){1.0, 0.0, 0.0});
	//e poi ruoto di 90 gradi il sistema di riferimento
	glm_rotate(modelViewMat, M_PI/2, (vec3){0.0, 1.0, 0.0});
	glm_scale(modelViewMat, (vec3){0.94, 1.0, 1.0}); //0.02+9.0+0.02 per dare l'effetto del rilievo	    	
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	//DISEGNO
	glUniform1ui(objectLoc, 5);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);

	// Finished drawing
    modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);

    
	// CENTRAL TOWER - WINDOW -------------------------------------------------------------------------------------------------------
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	// POSITIONING x=3.1+0.4   y=5.0+1.3+0.1+0.1 
	glm_translate(modelViewMat, (vec3){3.5, 6.5, -2.59}); //metto la finestra appena sopra al muro distanziando il quadrato su cui è dì 0.01 dalla parete
	glm_scale(modelViewMat, (vec3){0.2, 0.3, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	// DRAW
	glUniform1ui(objectLoc, 6);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	
	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);
    
	    	

	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);// finished drawing the castle




	/*I draw the base on which the model rests and the background*/
	    
	
	//CASTLE BASE *****************************************************************************************************************
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	// POSITIONING
	glm_translate(modelViewMat, (vec3){-8.0, 0.0, 8.0});
	glm_rotate(modelViewMat, -M_PI*90/180, (vec3){1.0, 0.0, 0.0});				
	glm_scale(modelViewMat, (vec3){23.0, 23.0, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	// DRAW
	glUniform1ui(objectLoc, 7);
	glBindVertexArray(vao[RECTANGLE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, RECTANGLE_VERTEX);
	
	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat);



	// BACKGROUND (starry sky)******************************************************************************************************
	modelViewMatStack = pushMatrix(modelViewMat, modelViewMatStack);

	// POSITIONING
	glm_translate(modelViewMat, (vec3){-8.0, 0.0, 8.0});
	glm_rotate(modelViewMat, M_PI/2, (vec3){0.0, 1.0, 0.0});
	glm_scale(modelViewMat, (vec3){23.0, 30.0, 1.0}); 
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	// DRAW
	glUniform1ui(objectLoc, 9);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);

	// POSITIONING
	glm_translate(modelViewMat, (vec3){1.0, 0.0, 0.0});
	glm_rotate(modelViewMat, -M_PI/2, (vec3){0.0, 1.0, 0.0});
	glm_scale(modelViewMat, (vec3){23.0, 1.0, 1.0});
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	// DRAW
	glUniform1ui(objectLoc, 9);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	    	
	// POSITIONING
	glm_translate(modelViewMat, (vec3){1.0, 0.0, 0.0});
	glm_rotate(modelViewMat, -M_PI/2, (vec3){0.0, 1.0, 0.0});
	glm_scale(modelViewMat, (vec3){1.0, 1.0, 1.0});
	glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, (GLfloat *)(modelViewMat));
	// Calculate and update normal matrix.
	glm_mat4_pick3(modelViewMat, TMP);
	glm_mat3_inv(TMP, normalMat);
	glm_mat3_transpose(normalMat);
	glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, (GLfloat *)normalMat);
	// DRAW
	glUniform1ui(objectLoc, 9);
	glBindVertexArray(vao[SQUARE]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, SQUARE_VERTEX);
	    
	// Finished drawing
	modelViewMatStack = popMatrix(modelViewMatStack, modelViewMat); 
	    	
	
	
    glutSwapBuffers();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27:
        exit(0);
        break;

	case 'f':
		if(rotazione_libera){
			rotazione_libera = 0;
			printf("\nFree rotation mode: OFF\n");
		} 
        else{
			rotazione_libera = 1;
			printf("\nFree rotation mode: ON");
			printf("\nPress 'x', 'X', 'y', 'Y', 'z', 'Z' to rotate the scene.\n");
		} 

		glutPostRedisplay();
        break;

    case 'x':
		if(rotazione_libera){
			Xangle += (2.0/360.0) * (2.0*M_PI);
        	if (Xangle > (2.0*M_PI)) Xangle -= 2.0*M_PI;
        	glutPostRedisplay();
		}
        break;

    case 'X':
		if(rotazione_libera){
        	Xangle -= (2.0/360.0) * (2.0*M_PI);
        	if (Xangle < 0.0) Xangle += 2.0*M_PI;
        	glutPostRedisplay();
		}
        break;
    case 'y':
		if(rotazione_libera){
        	Yangle += (2.0/360.0) * (2.0*M_PI);
        	if (Yangle > 2.0*M_PI) Yangle -= 2.0*M_PI;
        	glutPostRedisplay();
		}
        break;
    case 'Y':
		if(rotazione_libera){
			Yangle -= (2.0/360.0) * (2.0*M_PI);
        	if (Yangle < 0.0) Yangle += 2.0*M_PI;
       	 	glutPostRedisplay();
		}
        break;
    case 'z':
		if(rotazione_libera){
        	Zangle += (2.0/360.0) * (2.0*M_PI);
    	    if (Zangle > 2.0*M_PI) Zangle -= 2.0*M_PI;
        	glutPostRedisplay();
		}
        break;
    case 'Z':
		if(rotazione_libera){
        	Zangle -= (2.0/360.0) * (2.0*M_PI);
        	if (Zangle < 0.0) Zangle += 2.0*M_PI;
        	glutPostRedisplay();
		}
        break;

    case 'r':
		printf("\nReset\n");
		animazione_finita = 0;
		rotazione_libera = 0;
		printf("Free rotation mode: OFF\n");
        Xangle=0.0;
        Yangle=0.0;
        Zangle=0.0;
        gateAngle = initial_gateAngle;
		wall_height = initial_wall_height;
		castle_height= initial_castle_height;
        glutPostRedisplay();
        break;

    case 'd':
		if(animazione_finita){
			if(gateAngle + (2.0/360.0) * (2.0*M_PI) <= 2.09){
    	
        		gateAngle += (2.0/360.0) * (2.0*M_PI);
        		glutPostRedisplay();
        	}	
		}
        break;

    case 'u':
		if(animazione_finita){
			if(gateAngle - (2.0/360.0) * (2.0*M_PI) >= 0.0){
    	
        		gateAngle -= (2.0/360.0) * (2.0*M_PI);
        		glutPostRedisplay();
        	}
        	else{

        		gateAngle = 0.0;
        		glutPostRedisplay();
        	}
		}
		
        break;

	case ' ':
		if(!animazione_finita){
			// Camera rotation
			if(Yangle >= -19.0/180*M_PI)
				Yangle -= (0.15/180*M_PI);
		
			// Appearance of the walls
			if((wall_height + 0.05) <= 0.0){
				wall_height += 0.05;
			}
			else{
				wall_height = 0.0;
			}
		
			// Appearance of the castle
			if(wall_height >= -1.7){ // just before the walls are fully raised
				if((castle_height + 0.1) <= 0.0){    	
        			castle_height += 0.1;
       		 	}
    			else{
        			castle_height = 0.0;
						if(gateAngle + (5.0/360.0) * (2.0*M_PI) <= 2.09){
    						gateAngle += (5.0/360.0) * (2.0*M_PI);
        				}
        				else{
							gateAngle = 2.09;
							animazione_finita = 1;
						}
				}
        	}
		}
			
		
		glutPostRedisplay();
    	
        break;
    
    default:
        break;
    }

}

// Main routine.
int main(int argc, char **argv)
{
    GLenum glErr;

    printf("Interaction: \n");
	printf("Long press 'space' for playing the animation\n");
	printf("Press 'r' for resetting animation\n");

    printf("\nAt the end of the animation: \n");
	printf("Hold 'd' (down) to lower the drawbridge\n");
    printf("Hold 'u' (up) to raise the drawbridge\n");

	printf("\nPress 'f' to activare the free rotation mode (debugging)\n");
	printf("In free rotation mode press 'x', 'X', 'y', 'Y', 'z', 'Z' to rotate the scene.\n");
	printf("\nPress 'esc' to terminate the program\n");
	
    
    glutInit(&argc, argv);
    glutInitContextVersion(4, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1920, 1080);
    glutInitWindowPosition(0, 0);

    glutCreateWindow("Computer Graphics Exam Project");
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyInput);
    glewInit();

    // ... it does not hurt to check that everything went OK
    if ((glErr = glGetError()) != 0)
    {
        printf("Error = %d \n", glErr);
        exit(-1);
    }

    setup();
    glutMainLoop();

    return 0;
}
