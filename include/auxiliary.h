//-----------------------------------------------------------------------------
#ifndef _AUXILIARY_H_
#define _AUXILIARY_H_

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
void pollInput(void) {
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
}

//-----------------------------------------------------------------------------
GLuint vertShader;
GLuint fragShader;
GLuint shaderProgram0;

//-----------------------------------------------------------------------------
void buildShaderProgram0(void) {
	// NOTE: source code for the vertex shader
	const GLchar* vertSource = {
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
	vertShader = glCreateShader(GL_VERTEX_SHADER);

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
		exit(-1);
	}

	// NOTE: source code for the fragment shader
	const GLchar* fragSource = {
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
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);

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
		exit(-1);
	}

	// NOTE: create a shader program out of the vertex and fragment shaders
	shaderProgram0 = glCreateProgram();

	// NOTE: attach the vertex and fragment shaders
	glAttachShader(shaderProgram0, vertShader);
	glAttachShader(shaderProgram0, fragShader);

	// NOTE: link the shader program
	glLinkProgram(shaderProgram0);

	// NTOE: get the status of linking the program
	glGetProgramiv(shaderProgram0, GL_LINK_STATUS, &status);

	// NOTE: if the program failed to link print the error
	if(status == GL_FALSE) {
		char buffer[512];
		glGetProgramInfoLog(shaderProgram0, 512, NULL, buffer);
		fprintf(stderr, "glLinkProgram: %s\n", buffer);
		exit(-1);
	}

	// NOTE: use the newly compiled shader program
	glUseProgram(shaderProgram0);
}

//-----------------------------------------------------------------------------
void startGame(void) {
	// NOTE: initialize SDL2 library
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		exit(-1);
	}

	// NOTE: get player one's controller
	if(SDL_NumJoysticks()>0) {
		if(SDL_IsGameController(0)) {
			p1_controller = SDL_GameControllerOpen(0);
		} else {
			// NOTE: something is wrong with selected controller
			fprintf(stderr, "SDL_GameControllerOpen: %s\n", SDL_GetError());
			exit(-1);
		}
	} else {
		// NOTE: currently requiring a controller connection
		printf("\nPlease connect a controller.\n\n");
		exit(-1);
	}

	// NOTE: create game window
	window = SDL_CreateWindow(
		"Prototype",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		2*640,
		2*480,
		SDL_WINDOW_OPENGL
	);

	if(window == NULL) {
		fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
		exit(-1);
	}

	// NOTE: create an OpenGL context for the window
	context = SDL_GL_CreateContext(window);

	if(context == NULL) {
		fprintf(stderr, "SDL_GL_CreateContext: %s\n", SDL_GetError());
		exit(-1);
	}

	// NOTE: set the buffer swap interval to get vsync
	if(SDL_GL_SetSwapInterval(1) != 0) {
		fprintf(stderr, "SDL_GL_SetSwapInterval: %s\n", SDL_GetError());
		exit(-1);
	}

	// NTOE: initialize the OpenGL library function calls
	glError = glewInit();

	if(glError != GLEW_OK) {
		fprintf(stderr, "glewInit: %s\n", glewGetErrorString(glError));
		exit(-1);
	}

	// NOTE: enable the depth test
	glEnable(GL_DEPTH_TEST);

	// NOTE: build the shader program
	buildShaderProgram0();
}

//-----------------------------------------------------------------------------
void endGame(void) {
	// NOTE: detach the vertex and fragment shaders from the one shader program
	glDetachShader(shaderProgram0, vertShader);
	glDetachShader(shaderProgram0, fragShader);

	// NOTE: delete the shader program
	glDeleteProgram(shaderProgram0);

	// NOTE: delete the compiled shaders from the GPU
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

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
}

//-----------------------------------------------------------------------------
size_t numLoadedTextures = 0;

//-----------------------------------------------------------------------------
GLenum getFreeTex(void) {
	switch(numLoadedTextures) {
		case 0x00: return GL_TEXTURE0;
		case 0x01: return GL_TEXTURE1;
		case 0x02: return GL_TEXTURE2;
		case 0x03: return GL_TEXTURE3;
		case 0x04: return GL_TEXTURE4;
		case 0x05: return GL_TEXTURE5;
		case 0x06: return GL_TEXTURE6;
		case 0x07: return GL_TEXTURE7;
		default: return GL_TEXTURE8;
	}
}

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
		if(lines[i][0]=='v' && lines[i][1]==' ') {
			// NOTE: parse the vertex and add it to the array

			(*verts)->num++;

			numPos += 3;
			(*verts)->pos = (GLfloat*) realloc((*verts)->pos, numPos*sizeof(GLfloat));

			GLfloat x = 0, y = 0, z = 0;
			sscanf(lines[i], "v %f %f %f", &x, &y, &z);

			(*verts)->pos[numPos-3] = x;
			(*verts)->pos[numPos-2] = y;
			(*verts)->pos[numPos-1] = z;

		} else if(lines[i][0]=='v' && lines[i][1]=='t' && lines[i][2]==' ') {
			// NOTE: parse the texture coordinates

			numTex += 2;
			(*verts)->tex = (GLfloat*) realloc((*verts)->tex, numTex*sizeof(GLfloat));

			GLfloat u = 0, v = 0;
			sscanf(lines[i], "vt %f %f", &u, &v);

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
	}

	for(i=0; i<n; i++) {
		free(lines[i]);
		lines[i] = NULL;
	}

	free(lines);
	lines = NULL;
}

#endif
