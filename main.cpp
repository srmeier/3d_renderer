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
	GLfloat* pos;   // (x,y,z)
	GLfloat* tex;   // (u,v)
	GLfloat* norm;  // (x,y,z)
	GLsizeiptr num; // total num of verts
} VertInfo;

typedef struct {
	GLuint* pos;    // 3 pos inds per
	GLuint* tex;    // 3 tex inds per
	GLuint* norm;   // 3 norm inds per
	GLsizeiptr num; // num of triangles
} IndInfo;

//-----------------------------------------------------------------------------
void loadObjFile(VertInfo** verts, IndInfo** inds, const char* filename) {
	if(*verts) {
		free((*verts)->pos);
		(*verts)->pos = NULL;
		free((*verts)->tex);
		(*verts)->tex = NULL;
		free((*verts)->norm);
		(*verts)->norm = NULL;

		(*verts)->num = 0;
	} else *verts = (VertInfo*) calloc(0x01, sizeof(VertInfo));

	if(*inds) {
		free((*inds)->pos);
		(*inds)->pos = NULL;
		free((*inds)->tex);
		(*inds)->tex = NULL;
		free((*inds)->norm);
		(*inds)->norm = NULL;
		
		(*inds)->num = 0;
	} else *inds = (IndInfo*) calloc(0x01, sizeof(IndInfo));

	size_t n = 0;
	char** lines = NULL;

	if(filename==NULL) return;
	FILE* fp = fopen(filename, "r");

	do {
		char c;
		int colInd = 0;

		lines = (char**) realloc(lines, ++n*sizeof(char*));
		lines[n-1] = NULL;

		do {
			fread(&c, sizeof(char), 1, fp);

			// NOTE: strip newlines
			if(c!='\n') {
				if(colInd>0) {
					// NOTE: strip extra spaces
					if(!(lines[n-1][colInd-1]==' ' && c==' ')) {
						lines[n-1] = (char*) realloc(lines[n-1], ++colInd*sizeof(char));
						lines[n-1][colInd-1] = c;
					}
				} else {
					lines[n-1] = (char*) realloc(lines[n-1], ++colInd*sizeof(char));
					lines[n-1][colInd-1] = c;
				}
			}
		} while(c!='\n');

		// NOTE: end with a null terminator
		lines[n-1] = (char*) realloc(lines[n-1], ++colInd*sizeof(char));
		lines[n-1][colInd-1] = '\0';

		if(lines[n-1][0]=='#') {
			// NOTE: ignore comments

			free(lines[n-1]);
			lines[n-1] = NULL;

			lines = (char**) realloc(lines, --n*sizeof(char*));
		} else if(lines[n-1][0]=='\0') {
			// NOTE: ignore blank lines

			free(lines[n-1]);
			lines[n-1] = NULL;

			lines = (char**) realloc(lines, --n*sizeof(char*));
		}
	} while(!feof(fp));

	fclose(fp);
	fp = NULL;

	size_t numPos = 0;
	size_t numTex = 0;

	int i;
	for(i=0; i<n; i++) {
		// NOTE: parse the file

		printf("%s\n", lines[i]);

		if(lines[i][0]=='v' && lines[i][1]==' ') {
			// NOTE: parse the vertex and add it to the array

			(*verts)->num++;

			numPos += 3;
			(*verts)->pos = (GLfloat*) realloc((*verts)->pos, numPos*sizeof(GLfloat));

			GLfloat x = 0, y = 0, z = 0;
			sscanf(lines[i], "v %f %f %f", &x, &y, &z);
			printf("[%f, %f, %f]\n", x, y, z);

			(*verts)->pos[numPos-3] = x;
			(*verts)->pos[numPos-2] = y;
			(*verts)->pos[numPos-1] = z;

		} else if(lines[i][0]=='v' && lines[i][1]=='t' && lines[i][2]==' ') {
			// NOTE: parse the texture coordinates

			numTex += 2;
			(*verts)->tex = (GLfloat*) realloc((*verts)->tex, numTex*sizeof(GLfloat));

			GLfloat u = 0, v = 0;
			sscanf(lines[i], "vt %f %f", &u, &v);
			printf("[%f, %f]\n", u, v);

			(*verts)->tex[numTex-2] = u;
			(*verts)->tex[numTex-1] = v;
			
		} else if(lines[i][0] == 'f' && lines[i][1] == ' ') {
			// NOTE: parse the face elements

			int c, num = 0;
			GLuint* indices = NULL;

			for(c=1; c<strlen(lines[i]); c++) {

				// TODO: will have to handle the case when normals are left out
				// or when texture indices are left out

				// TODO: currently I am assuming 4 indices per face which makes
				// two triangles (i.e. 6 elements)

				if(lines[i][c]==' ') {
					num += 3;
					indices = (GLuint*) realloc(indices, num*sizeof(GLuint));

					GLuint vi = 0, vti = 0, vni = 0;
					sscanf(&lines[i][c], " %d/%d/%d", &vi, &vti, &vni);
					printf("[%d, %d, %d]\n", vi, vti, vni);

					indices[num-3] = vi;
					indices[num-2] = vti;
					indices[num-1] = vni;
				}
			}

			if(num==3*3) {
				// NOTE: triangle

				// TODO: currently only setting the position index but I need
				// to set them all eventually

				(*inds)->num += 3;

				(*inds)->pos = (GLuint*) realloc((*inds)->pos, (*inds)->num*sizeof(GLuint));
				(*inds)->tex = (GLuint*) realloc((*inds)->tex, (*inds)->num*sizeof(GLuint));
				(*inds)->norm = (GLuint*) realloc((*inds)->norm, (*inds)->num*sizeof(GLuint));

				int pos0 = indices[0];
				int pos1 = indices[3];
				int pos2 = indices[6];

				(*inds)->pos[(*inds)->num-3] = pos0-1;
				(*inds)->pos[(*inds)->num-2] = pos1-1;
				(*inds)->pos[(*inds)->num-1] = pos2-1;

				int tex0 = indices[1];
				int tex1 = indices[4];
				int tex2 = indices[7];

				(*inds)->tex[(*inds)->num-3] = tex0-1;
				(*inds)->tex[(*inds)->num-2] = tex1-1;
				(*inds)->tex[(*inds)->num-1] = tex2-1;

				int norm0 = indices[2];
				int norm1 = indices[5];
				int norm2 = indices[8];

				(*inds)->norm[(*inds)->num-3] = norm0-1;
				(*inds)->norm[(*inds)->num-2] = norm1-1;
				(*inds)->norm[(*inds)->num-1] = norm2-1;

			} else if(num==4*3) {
				// NOTE: quadrilateral

				// TODO: currently only setting the position index but I need
				// to set them all eventually

				(*inds)->num += 6;

				(*inds)->pos = (GLuint*) realloc((*inds)->pos, (*inds)->num*sizeof(GLuint));
				(*inds)->tex = (GLuint*) realloc((*inds)->tex, (*inds)->num*sizeof(GLuint));
				(*inds)->norm = (GLuint*) realloc((*inds)->norm, (*inds)->num*sizeof(GLuint));

				int pos0 = indices[0];
				int pos1 = indices[3];
				int pos2 = indices[6];
				int pos3 = indices[9];

				(*inds)->pos[(*inds)->num-6] = pos0-1;
				(*inds)->pos[(*inds)->num-5] = pos1-1;
				(*inds)->pos[(*inds)->num-4] = pos2-1;
				(*inds)->pos[(*inds)->num-3] = pos2-1;
				(*inds)->pos[(*inds)->num-2] = pos3-1;
				(*inds)->pos[(*inds)->num-1] = pos0-1;

				int tex0 = indices[1];
				int tex1 = indices[4];
				int tex2 = indices[7];
				int tex3 = indices[10];

				(*inds)->tex[(*inds)->num-6] = tex0-1;
				(*inds)->tex[(*inds)->num-5] = tex1-1;
				(*inds)->tex[(*inds)->num-4] = tex2-1;
				(*inds)->tex[(*inds)->num-3] = tex2-1;
				(*inds)->tex[(*inds)->num-2] = tex3-1;
				(*inds)->tex[(*inds)->num-1] = tex0-1;

				int norm0 = indices[2];
				int norm1 = indices[5];
				int norm2 = indices[8];
				int norm3 = indices[11];

				(*inds)->norm[(*inds)->num-6] = norm0-1;
				(*inds)->norm[(*inds)->num-5] = norm1-1;
				(*inds)->norm[(*inds)->num-4] = norm2-1;
				(*inds)->norm[(*inds)->num-3] = norm2-1;
				(*inds)->norm[(*inds)->num-2] = norm3-1;
				(*inds)->norm[(*inds)->num-1] = norm0-1;
			}

			free(indices);
			indices = NULL;
		}

		printf("\n");
	}

	for(i=0; i<n; i++) {
		free(lines[i]);
		lines[i] = NULL;
	}

	free(lines);
	lines = NULL;
}

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

	IndInfo* inds = NULL;
	VertInfo* verts = NULL;

	loadObjFile(&verts, &inds, argv[1]);
	//loadObjFile(&verts, &inds, argv[1]);

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

	GLfloat glVerts[5*inds->num];
	memset(glVerts, 0x00, 5*inds->num*sizeof(GLfloat));

	int i;
	for(i=0; i<inds->num; i++) {
		glVerts[5*i+0] = verts->pos[3*inds->pos[i]+0];
		glVerts[5*i+1] = verts->pos[3*inds->pos[i]+1];
		glVerts[5*i+2] = verts->pos[3*inds->pos[i]+2];

		glVerts[5*i+3] = verts->tex[2*inds->tex[i]+0];
		glVerts[5*i+4] = verts->tex[2*inds->tex[i]+1];
	}

	/*
	GLfloat glVerts[5*verts->num];
	memset(glVerts, 0x00, 5*verts->num*sizeof(GLfloat));

	// NOTE: vertices for a triangle (clockwise)
	int tempInd = 0;
	for(; tempInd<verts->num; tempInd++) {
		glVerts[5*tempInd+0] = verts->pos[3*tempInd+0];
		glVerts[5*tempInd+1] = verts->pos[3*tempInd+1];
		glVerts[5*tempInd+2] = verts->pos[3*tempInd+2];

		// NOTE: find the vertex index in the vert index array
		int i;
		GLuint temp = 0;
		SDL_bool first = SDL_TRUE;
		for(i=0; i<inds->num; i++) {
			if(inds->pos[i] == tempInd) {
				// NOTE: once the index into the vert index array is found
				// pull the corresponding texture info

				glVerts[5*tempInd+3] = verts->tex[2*inds->tex[i]+0];
				glVerts[5*tempInd+4] = verts->tex[2*inds->tex[i]+1];
				//break;
				if(first) {
					temp = inds->tex[i];
					first = SDL_FALSE;
				} else {
					if(temp != inds->tex[i]) printf("NOOOOOOOOOO\n");
				}
			}
		}
	}
	*/

	// NOTE: allocate an array buffer on the GPU
	GLuint verBuffer;
	glGenBuffers(1, &verBuffer);

	// NOTE: bind to the array buffer so that we may send our data to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, verBuffer);

	// NOTE: send our vertex data to the GPU and set as STAIC
	glBufferData(GL_ARRAY_BUFFER, 5*inds->num*sizeof(GLfloat), glVerts, GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, 5*verts->num*sizeof(GLfloat), glVerts, GL_STATIC_DRAW);

	// NOTE: get a pointer to the position attribute variable in the shader
	// program
	GLint posAttrib = glGetAttribLocation(shaderProgram, "pos");

	// NOTE: specify the stride (spacing) and offset for array buffer which
	// will be used in place of the attribute variable in the shader program
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
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
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));

	// NOTE: enable the attribute
	glEnableVertexAttribArray(texAttrib);

	/* END SET VERTEX INFORMATION */
	// ========================================================================

	/* SET ELEMENT INFORMATION */
	// ========================================================================

	// NOTE: index into the raw vertex array
	// inds->pos

	// NOTE: allocate a GPU buffer for the element data
	GLuint eleBuffer;
	glGenBuffers(1, &eleBuffer);

	// NOTE: bind to the element buffer so that we may send our data to the GPU
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBuffer);

	// NOTE: send our element data to the GPU and set as STAIC
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds->num*sizeof(GLuint), inds->pos, GL_STATIC_DRAW);

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
	SDL_Surface *surface = SDL_LoadBMP("tex02.bmp");

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
	model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

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
		glDrawArrays(GL_TRIANGLES, 0, inds->num);
		//glDrawElements(GL_TRIANGLES, inds->num, GL_UNSIGNED_INT, 0);

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
