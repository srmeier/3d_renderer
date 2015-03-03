/* Prototype
g++ main.cpp -o test.exe -I./include -L./lib -lmingw32 -lglew32 -lopengl32 -lSDL2main -lSDL2
running on GCC 4.8.1, SDL 2.0.1, GLEW 1.10.0, and GLM 0.9.6.1
*/

//-----------------------------------------------------------------------------
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "GL/glew.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

//-----------------------------------------------------------------------------
typedef struct {
	int n;
	char** lines;
	char* filename;
} ObjFile;

void loadObjFile(ObjFile* objfile) {
	if(objfile->lines) {
		int i;
		for(i=0; i<objfile->n; i++) {
			free(objfile->lines[i]);
		} free(objfile->lines);
	}

	objfile->n = 0;
	objfile->lines = NULL;

	if(objfile->filename==NULL) return;
	FILE* fp = fopen(objfile->filename, "r");

	do {
		char c;
		int colInd = 0;

		objfile->lines = (char**) realloc(objfile->lines, ++objfile->n*sizeof(char*));
		objfile->lines[objfile->n-1] = NULL;

		do {
			fread(&c, sizeof(char), 1, fp);

			// NOTE: strip newlines
			if(c!='\n') {
				if(colInd>0) {
					// NOTE: strip extra spaces
					if(!(objfile->lines[objfile->n-1][colInd-1]==' ' && c==' ')) {
						objfile->lines[objfile->n-1] = (char*) realloc(objfile->lines[objfile->n-1], ++colInd*sizeof(char));
						objfile->lines[objfile->n-1][colInd-1] = c;
					}
				} else {
					objfile->lines[objfile->n-1] = (char*) realloc(objfile->lines[objfile->n-1], ++colInd*sizeof(char));
					objfile->lines[objfile->n-1][colInd-1] = c;
				}
			}
		} while(c!='\n');

		// NOTE: end with a null terminator
		objfile->lines[objfile->n-1] = (char*) realloc(objfile->lines[objfile->n-1], ++colInd*sizeof(char));
		objfile->lines[objfile->n-1][colInd-1] = '\0';

		if(objfile->lines[objfile->n-1][0]=='#') {
			// NOTE: ignore comments

			free(objfile->lines[objfile->n-1]);
			objfile->lines[objfile->n-1] = NULL;

			objfile->lines = (char**) realloc(objfile->lines, --objfile->n*sizeof(char*));
		} else if(objfile->lines[objfile->n-1][0]=='\0') {
			// NOTE: ignore blank lines

			free(objfile->lines[objfile->n-1]);
			objfile->lines[objfile->n-1] = NULL;

			objfile->lines = (char**) realloc(objfile->lines, --objfile->n*sizeof(char*));
		}
	} while(!feof(fp));

	fclose(fp);
	fp = NULL;
}

//-----------------------------------------------------------------------------
/*
typedef struct {
} Mesh;

void loadMesh(Mesh* mesh, ObjFile* objfile) {
	int n[4] = {};

	int* indices = NULL;
	float* vertsPos = NULL;
	float* vertsTex = NULL;

	//int numVerts = 0;
	//float* verts = NULL;

	//int numTexsco = 0;
	//float* texsco = NULL;

	//int numIndices = 0;
	//int* indices = NULL;

	//int numFaces = 0;

	int i;
	for(i=0; i<objfile->n; i++) {
		// NOTE: parse the file

		if(objfile->lines[i][0] == 'v' && objfile->lines[i][1] == ' ') {
			// NOTE: parse the vertex and add it to the array

			numVerts += 3;
			verts = (float *)realloc(verts, numVerts*sizeof(float));

			float x = 0, y = 0, z = 0;
			sscanf(objfile->lines[i], "v %f %f %f", &x, &y, &z);

			verts[numVerts-3] = x;
			verts[numVerts-2] = y;
			verts[numVerts-1] = z;

		} else if(objfile->lines[i][0] == 'v' && objfile->lines[i][1] == 't' && objfile->lines[i][2] == ' ') {
			// NOTE: parse the texture coordinates

			numTexsco += 3;
			texsco = (float *)realloc(texsco, numTexsco*sizeof(float));

			float x = 0, y = 0, z = 0;
			sscanf(objfile->lines[i], "vt %f %f %f", &x, &y, &z);

			texsco[numTexsco-3] = x;
			texsco[numTexsco-2] = y;
			texsco[numTexsco-1] = z;
			
		} else if(objfile->lines[i][0] == 'f' && objfile->lines[i][1] == ' ') {
			// NOTE: parse the face elements
			numFaces++;

			int c;
			for(c=1; c<strlen(objfile->lines[i]); c++) {

				int vi = 0, vti = 0, vni = 0;
				if(objfile->lines[i][c] == ' ') {
					numIndices += 3;
					indices = (int *)realloc(indices, numIndices*sizeof(int));

					sscanf(&objfile->lines[i][c], " %d/%d/%d", &vi, &vti, &vni);

					indices[numIndices-3] = vi;
					indices[numIndices-2] = vti;
					indices[numIndices-1] = vni;
				}

				// TODO: will have to handle the case when normals are left out
				// or when texture indices are left out
			}
		}
	}
}
*/

//-----------------------------------------------------------------------------
typedef struct {
	SDL_bool up;
	int m_x, m_y;
	SDL_bool down;
	SDL_bool left;
	SDL_bool right;
	SDL_bool m_left;
	SDL_bool m_right;
	SDL_bool w, a, s, d;
} Input;

//-----------------------------------------------------------------------------
Input pl_input;
GLenum glError;
SDL_Event event;
SDL_bool running;
SDL_Window *window;
SDL_GLContext context;
SDL_GameController *p1_controller;

//-----------------------------------------------------------------------------
int SDL_main(int argc, char *argv[]) {
	// NOTE: initialize SDL2 library
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		return -1;
	}

	// NOTE: get player one's controller
	if(SDL_NumJoysticks()>0) {
		if(SDL_IsGameController(0)) {
			p1_controller = SDL_GameControllerOpen(0);
		} else {
			// NOTE: something is wrong with selected controller
			fprintf(stderr, "SDL_GameControllerOpen: %s\n", SDL_GetError());
			return -1;
		}
	} else {
		// NOTE: currently requiring a controller connection
		printf("\nPlease connect a controller.\n\n");
		//return 0;
	}

	// NOTE: create game window
	window = SDL_CreateWindow(
		"Prototype",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640,
		480,
		SDL_WINDOW_OPENGL
	);

	if(window == NULL) {
		fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
		return -1;
	}

	// NOTE: create an OpenGL context for the window
	context = SDL_GL_CreateContext(window);

	if(context == NULL) {
		fprintf(stderr, "SDL_GL_CreateContext: %s\n", SDL_GetError());
		return -1;
	}

	// NOTE: set the buffer swap interval to get vsync
	if(SDL_GL_SetSwapInterval(1) != 0) {
		fprintf(stderr, "SDL_GL_SetSwapInterval: %s\n", SDL_GetError());
		return -1;
	}

	// NTOE: initialize the OpenGL library function calls
	glError = glewInit();

	if(glError != GLEW_OK) {
		fprintf(stderr, "glewInit: %s\n", glewGetErrorString(glError));
		return -1;
	}

	// NOTE: enable the depth test
	glEnable(GL_DEPTH_TEST);

	/* SET SHADER PROGRAM */
	// ========================================================================

	// NOTE: source code for the vertex shader
	const GLchar *vertSource = {
		"#version 150 core\n"\
		"\n"\
		"in vec3 pos;\n"\
		"in vec2 texcoord;\n"\
		"\n"\
		"out vec2 fragTexcoord;\n"\
		"\n"\
		"uniform mat4 model;\n"\
		"uniform mat4 view;\n"\
		"uniform mat4 proj;\n"\
		"\n"\
		"void main() {\n"\
			"fragTexcoord = texcoord;\n"\
			"gl_Position = proj*view*model*vec4(pos, 1.0);\n"\
		"}\n"\
		"\0"
	};

	// NOTE: allocate vertex shader program
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);

	// NOTE: load the vertex shader's source code
	glShaderSource(vertShader, 1, &vertSource, NULL);

	// NOTE: compile the vertex shader's source code
	glCompileShader(vertShader);

	// NOTE: get the status of the compilation
	GLint status;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &status);

	// NOTE: if the compilation failed print the error
	if(status == GL_FALSE) {
		char buffer[512];
		glGetShaderInfoLog(vertShader, 512, NULL, buffer);
		fprintf(stderr, "glCompileShader: %s\n", buffer);
		return -1;
	}

	// NOTE: source code for the fragment shader
	const GLchar *fragSource = {
		"#version 150 core\n"\
		"\n"\
		"in vec2 fragTexcoord;\n"\
		"uniform sampler2D tex;\n"\
		"\n"\
		"void main() {\n"\
			"gl_FragColor = texture(tex, fragTexcoord)*vec4(1.0, 1.0, 1.0, 1.0);\n"\
		"}\n"\
		"\0"
	};

	// NOTE: allocate fragment shader program
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	// NOTE: load the fragment shader's source code
	glShaderSource(fragShader, 1, &fragSource, NULL);

	// NOTE: compile the vertex shader's source code
	glCompileShader(fragShader);

	// NOTE: get the status of the compilation
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);

	// NOTE: if the compilation failed print the error
	if(status == GL_FALSE) {
		char buffer[512];
		glGetShaderInfoLog(fragShader, 512, NULL, buffer);
		fprintf(stderr, "glCompileShader: %s\n", buffer);
		return -1;
	}

	// NOTE: create a shader program out of the vertex and fragment shaders
	GLuint shaderProgram = glCreateProgram();

	// NOTE: attach the vertex and fragment shaders
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);

	// NOTE: link the shader program
	glLinkProgram(shaderProgram);

	// NTOE: get the status of linking the program
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);

	// NOTE: if the program failed to link print the error
	if(status == GL_FALSE) {
		char buffer[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, buffer);
		fprintf(stderr, "glLinkProgram: %s\n", buffer);
		return -1;
	}

	// NOTE: use the newly compiled shader program
	glUseProgram(shaderProgram);

	/* END SET SHADER PROGRAM */
	// ========================================================================

	/* BEGIN TEST OBJ FILE PARSING */
	// ========================================================================

	// NOTE: check for the correct number of arguements
	if(argc<2) {
		// NOTE: need a .obj file to test
		fprintf(stderr, "\nNeed an object file for testing.\n\n");
		return 0;
	}

	ObjFile objfile = {};
	objfile.filename = argv[1];
	loadObjFile(&objfile);



	int numVerts = 0;
	float *verts = NULL;

	int numTexsco = 0;
	float *texsco = NULL;

	int numIndices = 0;
	int *indices = NULL;

	int numFaces = 0;

	int i, j;
	for(j=0; j<objfile.n; j++) {
		// NOTE: parse the file

		printf("%s\n", objfile.lines[j]);

		if(objfile.lines[j][0] == 'v' && objfile.lines[j][1] == ' ') {
			// NOTE: parse the vertex and add it to the array

			numVerts += 3;
			verts = (float *)realloc(verts, numVerts*sizeof(float));

			float x = 0, y = 0, z = 0;
			sscanf(objfile.lines[j], "v %f %f %f", &x, &y, &z);
			printf("[%f, %f, %f]\n", x, y, z);

			verts[numVerts-3] = x;
			verts[numVerts-2] = y;
			verts[numVerts-1] = z;

		} else if(objfile.lines[j][0] == 'v' && objfile.lines[j][1] == 't' && objfile.lines[j][2] == ' ') {
			// NOTE: parse the texture coordinates

			numTexsco += 3;
			texsco = (float *)realloc(texsco, numTexsco*sizeof(float));

			float x = 0, y = 0, z = 0;
			sscanf(objfile.lines[j], "vt %f %f %f", &x, &y, &z);
			printf("[%f, %f, %f]\n", x, y, z);

			texsco[numTexsco-3] = x;
			texsco[numTexsco-2] = y;
			texsco[numTexsco-1] = z;
			
		} else if(objfile.lines[j][0] == 'f' && objfile.lines[j][1] == ' ') {
			// NOTE: parse the face elements
			numFaces++;

			int c;
			for(c=1; c<strlen(objfile.lines[j]); c++) {

				int vi = 0, vti = 0, vni = 0;
				if(objfile.lines[j][c] == ' ') {
					numIndices += 3;
					indices = (int *)realloc(indices, numIndices*sizeof(int));

					sscanf(&objfile.lines[j][c], " %d/%d/%d", &vi, &vti, &vni);
					printf("[%d, %d, %d]\n", vi, vti, vni);

					indices[numIndices-3] = vi;
					indices[numIndices-2] = vti;
					indices[numIndices-1] = vni;
				}

				// TODO: will have to handle the case when normals are left out
				// or when texture indices are left out
			}
		}

		printf("\n");
	}

	/*
	for(i=0; i<numl; i++) {
		free(lines[i]);
	}

	free(lines);

	// NOTE: close the file
	fclose(obj_file);
	obj_file = NULL;
	*/

	/* END TEST OBJ FILE PARSING */
	// ========================================================================

	/* SET VERTEX INFORMATION */
	// ========================================================================

	// NOTE: create a vertex array object to store all the relationships
	// between vertex buffer objects and shader program attributes
	GLuint verArray;
	glGenVertexArrays(1, &verArray);

	// NOTE: bind to this vertex array when establishing all the connections
	// for the shaderProgram
	glBindVertexArray(verArray);
	/*
	- if I every need to switch raw vertex data to program attributes all I
		need to do is bind a different vertex array object
	*/

	float vertices[5*(numVerts/3)];
	memset(vertices, 0x00, sizeof(float)*5*(numVerts/3));

	int tempIndex = 0;
	for(; tempIndex<(numVerts/3); tempIndex++) {
		vertices[5*tempIndex+0] = verts[3*tempIndex+0];
		vertices[5*tempIndex+1] = verts[3*tempIndex+1];
		vertices[5*tempIndex+2] = verts[3*tempIndex+2];

		vertices[5*tempIndex+3] = texsco[3*tempIndex+0];
		vertices[5*tempIndex+4] = texsco[3*tempIndex+1];
	}

	// NOTE: vertices for a triangle (clockwise)
	/*
	float vertices[] = {
		-0.5f,  0.5f, 0.0f, 0.0f, 0.0f,
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f
	};
	float vertices[] = {
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, 1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, 0.0f, 1.0f
	};
	*/

	// NOTE: allocate an array buffer on the GPU
	GLuint verBuffer;
	glGenBuffers(1, &verBuffer);

	// NOTE: bind to the array buffer so that we may send our data to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, verBuffer);

	// NOTE: send our vertex data to the GPU and set as STAIC
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// NOTE: get a pointer to the position attribute variable in the shader
	// program
	GLint posAttrib = glGetAttribLocation(shaderProgram, "pos");

	// NOTE: specify the stride (spacing) and offset for array buffer which
	// will be used in place of the attribute variable in the shader program
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
	/*
	- this function call automatically directs the array buffer bound to
		GL_ARRAY_BUFFER towards the attribute in the shader program
	*/

	// NOTE: enable the attribute
	glEnableVertexAttribArray(posAttrib);

	// NOTE: get a pointer to the position attribute variable in the shader
	// program
	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");

	// NOTE: specify the stride (spacing) and offset for array buffer which
	// will be used in place of the attribute variable in the shader program
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)(3*sizeof(float)));

	// NOTE: enable the attribute
	glEnableVertexAttribArray(texAttrib);

	/* END SET VERTEX INFORMATION */
	// ========================================================================

	/* SET ELEMENT INFORMATION */
	// ========================================================================

	// NOTE: index into the raw vertex array

	GLuint elements[6*numFaces];
	memset(elements, 0x00, sizeof(GLuint)*6*numFaces);


	tempIndex = 0;
	for(; tempIndex<numFaces; tempIndex++) {
		int ind0 = indices[4*3*tempIndex+0];
		int ind1 = indices[4*3*tempIndex+3];
		int ind2 = indices[4*3*tempIndex+6];
		int ind3 = indices[4*3*tempIndex+9];

		elements[6*tempIndex+0] = ind0-1;
		elements[6*tempIndex+1] = ind1-1;
		elements[6*tempIndex+2] = ind2-1;
		elements[6*tempIndex+3] = ind2-1;
		elements[6*tempIndex+4] = ind3-1;
		elements[6*tempIndex+5] = ind0-1;
	}

	/*
	GLuint elements[] = {
		0, 1, 2,
		2, 3, 0
	};
	*/

	// NOTE: allocate a GPU buffer for the element data
	GLuint eleBuffer;
	glGenBuffers(1, &eleBuffer);

	// NOTE: bind to the element buffer so that we may send our data to the GPU
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBuffer);

	// NOTE: send our element data to the GPU and set as STAIC
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	/* END SET ELEMENT INFORMATION */
	// ========================================================================

	/* SET TEXTURE INFORMATION */
	// ========================================================================

	// NOTE: allocate a GPU texture
	GLuint tex;
	glGenTextures(1, &tex);

	// NOTE: set the texture to unit 0
	glActiveTexture(GL_TEXTURE0);

	// NOTE: bind to the texture so that we may send our data to the GPU
	glBindTexture(GL_TEXTURE_2D, tex);

	// NOTE: load the bitmap SDL surface for texture pixels
	SDL_Surface *surface = SDL_LoadBMP("tex01.bmp");

	if(surface == NULL) {
		fprintf(stderr, "SDL_LoadBMP: %s\n", SDL_GetError());
		return -1;
	}

	// NOTE: convert SDL2 surface by swapping RGBA locations
	SDL_PixelFormat newFormat = *surface->format;

	newFormat.Rmask = 0x000000FF;
	newFormat.Gmask = 0x0000FF00;
	newFormat.Bmask = 0x00FF0000;
	newFormat.Amask = 0xFF000000;

	// NOTE: get the converted surface
	SDL_Surface *nSurface = SDL_ConvertSurface(surface, &newFormat, 0);

	// NOTE: send the pixels to the GPU
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, nSurface->pixels);

	// NOTE: bind the uniform "tex" in the fragment shader to the unit 0
	// texture
	glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);

	// NOTE: generate the mipmaps for scaling
	glGenerateMipmap(GL_TEXTURE_2D);

	// NOTE: free the converted surface
	SDL_FreeSurface(nSurface);
	nSurface = NULL;

	// NOTE: free the original surface	
	SDL_FreeSurface(surface);
	surface = NULL;

	/* END SET TEXTURE INFORMATION */
	// ========================================================================

	/* TESTING */
	// ========================================================================

	glm::mat4 model;
	float angle = (float) M_PI / 500.0f;
	//model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

	GLint uniModel = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	float pDist = 4.2f;
	glm::mat4 view = glm::lookAt(
		glm::vec3(pDist, pDist, pDist),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	GLint uniView = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 proj = glm::perspective(45.0f, 800.0f / 600.0f, 1.0f, 256.0f);

	GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	/* END TESTING */
	// ========================================================================

	// NOTE: start running the game
	running = SDL_TRUE;

	while(running) {
		// NOTE: poll for player input
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				// NOTE: if the player clicks on the X button close the window
				case SDL_QUIT: {
					running = SDL_FALSE;
				} break;

				// NOTE: check the location of the mouse pointer
				case SDL_MOUSEMOTION: {
					pl_input.m_x = event.motion.x;
					pl_input.m_y = event.motion.y;
				} break;

				// NOTE: check the mouse buttons are up
				case SDL_MOUSEBUTTONUP: {
					switch(event.button.button) {
						case SDL_BUTTON_LEFT: {
							pl_input.m_left = SDL_FALSE;
						} break;
						case SDL_BUTTON_RIGHT: {
							pl_input.m_right = SDL_FALSE;
						} break;
					}
				} break;

				// NOTE: check the mouse buttons are down
				case SDL_MOUSEBUTTONDOWN: {
					switch(event.button.button) {
						case SDL_BUTTON_LEFT: {
							pl_input.m_left = SDL_TRUE;
						} break;
						case SDL_BUTTON_RIGHT: {
							pl_input.m_right = SDL_TRUE;
						} break;
					}
				} break;

				// NOTE: set any pressed keys to the down position
				case SDL_KEYDOWN: {
					switch(event.key.keysym.sym) {
						case SDLK_ESCAPE: running = SDL_FALSE; break;
						case SDLK_w: pl_input.w = SDL_TRUE; break;
						case SDLK_a: pl_input.a = SDL_TRUE; break;
						case SDLK_s: pl_input.s = SDL_TRUE; break;
						case SDLK_d: pl_input.d = SDL_TRUE; break;
						case SDLK_UP: pl_input.up = SDL_TRUE; break;
						case SDLK_DOWN: pl_input.down = SDL_TRUE; break;
						case SDLK_LEFT: pl_input.left = SDL_TRUE; break;
						case SDLK_RIGHT: pl_input.right = SDL_TRUE; break;
					}
				} break;

				// NOTE: set any keys that aren't pressed to the up position
				case SDL_KEYUP: {
					switch(event.key.keysym.sym) {
						case SDLK_w: pl_input.w = SDL_FALSE; break;
						case SDLK_a: pl_input.a = SDL_FALSE; break;
						case SDLK_s: pl_input.s = SDL_FALSE; break;
						case SDLK_d: pl_input.d = SDL_FALSE; break;
						case SDLK_UP: pl_input.up = SDL_FALSE; break;
						case SDLK_DOWN: pl_input.down = SDL_FALSE; break;
						case SDLK_LEFT: pl_input.left = SDL_FALSE; break;
						case SDLK_RIGHT: pl_input.right = SDL_FALSE; break;
					}
				} break;
			}
		}

		if(pl_input.up) {
			pDist -= 0.05f;
			glm::mat4 view = glm::lookAt(
				glm::vec3(pDist, pDist, pDist),
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f)
			);

			GLint uniView = glGetUniformLocation(shaderProgram, "view");
			glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
		}

		if(pl_input.down) {
			pDist += 0.05f;
			glm::mat4 view = glm::lookAt(
				glm::vec3(pDist, pDist, pDist),
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f)
			);

			GLint uniView = glGetUniformLocation(shaderProgram, "view");
			glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
		}

		if(pl_input.left) {
			model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
		}

		if(pl_input.right) {
			model = glm::rotate(model, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
		}

		uint8_t b_bnt = SDL_GameControllerGetButton(p1_controller, SDL_CONTROLLER_BUTTON_B);
		if(b_bnt) running = SDL_FALSE;

		int x_axis = SDL_GameControllerGetAxis(p1_controller, SDL_CONTROLLER_AXIS_LEFTX);
		int y_axis = SDL_GameControllerGetAxis(p1_controller, SDL_CONTROLLER_AXIS_LEFTY);

		// NOTE: clear the screen buffer
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		/* TESTING */
		// ====================================================================
		// NOTE: test spinning
		if(abs(x_axis)>10000)
			model = glm::rotate(model, (float) x_axis/1000000.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		if(abs(y_axis)>10000)
			model = glm::rotate(model, (float) y_axis/1000000.0f, glm::vec3(0.0f, 1.0f, 0.0f));

		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

		// NOTE: draw to the screen
		//glDrawArrays(GL_TRIANGLES, 0, (4*numFaces));
		//glDrawArrays(GL_TRIANGLES, 0, 36);

		glDrawElements(GL_TRIANGLES, 6*numFaces, GL_UNSIGNED_INT, 0);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		/* END TESTING */
		// ====================================================================

		// NOTE: swap the front and back buffers
		SDL_GL_SwapWindow(window);
	}

	/* TESTING */
	// ========================================================================

	// NOTE: free the GPU texture
	glDeleteTextures(1, &tex);

	// NOTE: free the arrays buffer on the GPU
	glDeleteBuffers(1, &eleBuffer);
	glDeleteBuffers(1, &verBuffer);

	// NOTE: free the vertex array attribute relation on the GPU
	glDeleteVertexArrays(1, &verArray);

	// NOTE: detach the vertex and fragment shaders from the one shader program
	glDetachShader(shaderProgram, vertShader);
	glDetachShader(shaderProgram, fragShader);

	// NOTE: delete the shader program
	glDeleteProgram(shaderProgram);

	// NOTE: delete the compiled shaders from the GPU
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	/* END TESTING */
	// ========================================================================

	// NOTE: free the OpenGL context
	SDL_GL_DeleteContext(context);
	context = NULL;
	
	// NOTE: free the SDL2 window
	SDL_DestroyWindow(window);
	window = NULL;

	// NOTE: free player one's controller
	SDL_GameControllerClose(p1_controller);

	// NOTE: quit the SDL2 library
	SDL_Quit();
	
	return 0;
}
