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
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "GL/glew.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengl.h"

#include "auxiliary.h"

//-----------------------------------------------------------------------------
GLuint shaderProgram;

//-----------------------------------------------------------------------------
class Mesh {
private:
	IndInfo* inds;
	VertInfo* verts;
	SDL_Surface* sdl_surf;

	bool glLoaded;
	GLuint vertArray;
	GLuint vertBuffer;
	GLuint textureInd;

protected:
	// transforms

public:
	Mesh(const char* obj_fn, const char* tex_fn);
	~Mesh(void);

	void render(void);

	void glBind(void);
	void glLoad(void);
	void glUnload(void);
};

//-----------------------------------------------------------------------------
Mesh::Mesh(const char* obj_fn, const char* tex_fn) {
	inds = NULL;
	verts = NULL;
	sdl_surf = NULL;

	loadObjFile(&verts, &inds, obj_fn);

	SDL_Surface* temp_surf = SDL_LoadBMP(tex_fn);

	SDL_PixelFormat nFormat = *temp_surf->format;

	nFormat.Rmask = 0x000000FF;
	nFormat.Gmask = 0x0000FF00;
	nFormat.Bmask = 0x00FF0000;
	nFormat.Amask = 0xFF000000;

	sdl_surf = SDL_ConvertSurface(temp_surf, &nFormat, 0);

	SDL_FreeSurface(temp_surf);
	temp_surf = NULL;

	glLoaded = false;
}

Mesh::~Mesh(void) {
	loadObjFile(&verts, &inds, NULL);

	SDL_FreeSurface(sdl_surf);

	inds = NULL;
	verts = NULL;
	sdl_surf = NULL;
}

void Mesh::render(void) {
	if(!glLoaded) return;

	//glBind();
	glDrawArrays(GL_TRIANGLES, 0, inds->num);
}

void Mesh::glBind(void) {
	if(!glLoaded) return;

	glBindVertexArray(vertArray);
	glBindTexture(GL_TEXTURE_2D, textureInd);
}

void Mesh::glLoad(void) {
	// NOTE: generate OpenGL variables
	glGenBuffers(1, &vertBuffer);
	glGenTextures(1, &textureInd);
	glGenVertexArrays(1, &vertArray);

	// NOTE: activate a free texture
	glActiveTexture(getFreeTex());

	// NOTE: bind to the OpenGL variables
	glBindVertexArray(vertArray);
	glBindTexture(GL_TEXTURE_2D, textureInd);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);

	// NOTE: generate the OpenGL vert information
	GLfloat glVerts[5*inds->num];
	memset(glVerts, 0x00, 5*inds->num*sizeof(GLfloat));

	for(int i=0; i<inds->num; i++) {
		glVerts[5*i+0] = verts->pos[3*inds->pos[i]+0];
		glVerts[5*i+1] = verts->pos[3*inds->pos[i]+1];
		glVerts[5*i+2] = verts->pos[3*inds->pos[i]+2];
		glVerts[5*i+3] = verts->tex[2*inds->tex[i]+0];
		glVerts[5*i+4] = verts->tex[2*inds->tex[i]+1];
	}

	// NOTE: pass the information to OpenGL
	glBufferData(GL_ARRAY_BUFFER, 5*inds->num*sizeof(GLfloat), glVerts, GL_STATIC_DRAW);

	// NOTE: pass the texture information to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sdl_surf->w, sdl_surf->h, 0, GL_RGB, GL_UNSIGNED_BYTE, sdl_surf->pixels);
	numLoadedTextures++;

	// NOTE: set the shader program variables
	glUniform1i(glGetUniformLocation(shaderProgram, "tex"), numLoadedTextures-1);
	glGenerateMipmap(GL_TEXTURE_2D);

	GLint posAttrib = glGetAttribLocation(shaderProgram, "pos");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
	glEnableVertexAttribArray(posAttrib);

	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrib);

	glLoaded = true;
}

void Mesh::glUnload(void) {
	glDeleteBuffers(1, &vertBuffer);
	glDeleteTextures(1, &textureInd);
	glDeleteVertexArrays(1, &vertArray);

	glLoaded = false;
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
		2*640,
		2*480,
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
	shaderProgram = glCreateProgram();

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

	/* TESTING */
	// ========================================================================

	float cBoxW = 95.0f;

	int swing_frame = 0;
	SDL_bool swinging = SDL_FALSE;
	SDL_bool swing_forward = SDL_TRUE;

	glm::vec3 m_position(0.0f, 0.0f, 0.0f);
	glm::vec3 m_direction(0.0f, 0.0f, 1.0f);

	glm::mat4 model;
	float angle = (float) M_PI/100.0f;
	model = glm::translate(model, glm::vec3(0.0f, 15.0f, 0.0f));

	GLint uniModel = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	float pDist = 0.0f;
	glm::mat4 view = glm::lookAt(
		glm::vec3(pDist, pDist, pDist),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	GLint uniView = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 proj = glm::perspective(45.0f, 800.0f/600.0f, 1.0f, 256.0f);

	GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	Mesh* roomMesh = new Mesh("test.obj", "tex02.bmp");
	roomMesh->glLoad();

	Mesh* testMesh = new Mesh("wall.obj", "tex02.bmp");
	testMesh->glLoad();

	Mesh* swordMesh = new Mesh("sword.obj", "tex00.bmp");
	swordMesh->glLoad();

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

		// NOTE: move around with left dpad
		if(pl_input.up) {
			m_position += m_direction;

			if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
				m_position -= m_direction;
		}

		if(pl_input.down) {
			m_position -= m_direction;

			if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
				m_position += m_direction;
		}

		if(pl_input.left) {
			m_direction = glm::rotate(m_direction, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		if(pl_input.right) {
			m_direction = glm::rotate(m_direction, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		// NOTE: clse on start button
		uint8_t start_bnt = SDL_GameControllerGetButton(p1_controller, SDL_CONTROLLER_BUTTON_START);
		if(start_bnt) running = SDL_FALSE;

		uint8_t y_bnt = SDL_GameControllerGetButton(p1_controller, SDL_CONTROLLER_BUTTON_Y);
		if(y_bnt && !swinging) {
			swing_frame = 0;
			swinging = SDL_TRUE;
			swing_forward = SDL_TRUE;
		}

		// NOTE: strafe on left trigger
		uint8_t l_trig = SDL_GameControllerGetButton(p1_controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
		if(l_trig) {
			m_position -= glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f));

			if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
				m_position += glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		// NOTE: strafe on right trigger
		uint8_t r_trig = SDL_GameControllerGetButton(p1_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
		if(r_trig) {
			m_position += glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f));

			if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
				m_position -= glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		// NOTE: get axis numbers
		int x_axisl = SDL_GameControllerGetAxis(p1_controller, SDL_CONTROLLER_AXIS_LEFTX);
		int y_axisl = SDL_GameControllerGetAxis(p1_controller, SDL_CONTROLLER_AXIS_LEFTY);

		int x_axisr = SDL_GameControllerGetAxis(p1_controller, SDL_CONTROLLER_AXIS_RIGHTX);
		int y_axisr = SDL_GameControllerGetAxis(p1_controller, SDL_CONTROLLER_AXIS_RIGHTY);

		// NOTE: easy movement with the left stick
		if(abs(x_axisl)>10000) {
			m_direction = glm::rotate(m_direction, (float) -x_axisl/1000000.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		}

		if(abs(y_axisl)>10000) {
			if(y_axisl>0) {
				m_position -= m_direction;

				if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
					m_position += m_direction;
			}

			if(y_axisl<0) {
				m_position += m_direction;

				if((m_position.x>cBoxW||m_position.x<-cBoxW) || (m_position.z>cBoxW||m_position.z<-cBoxW))
					m_position -= m_direction;
			}
		}

		// NOTE: clear the screen buffer
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		/* TESTING */
		// ====================================================================

		roomMesh->glBind();

		view = glm::lookAt(m_position, m_position+m_direction, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
		
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

		roomMesh->render();

		// NOTE: draw sword
		swordMesh->glBind();

		// NOTE: sword transform
		if((swinging&&swing_forward) && swing_frame<60) {
			swing_frame+=2;
		} else if((swinging&&!swing_forward) && swing_frame>0) {
			swing_frame-=2;
		} else {
			if(swing_forward) {
				swing_forward = SDL_FALSE;
			} else {
				swing_frame = 0;
				swinging = SDL_FALSE;
				swing_forward = SDL_TRUE;
			}
		}

		glm::mat4 smodel;
		smodel = glm::translate(smodel, m_position);
		float sangle = glm::sign(m_direction.x)*glm::angle(glm::normalize(m_direction), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::vec3 spos = glm::rotate(glm::vec3(-5.0f+(float)swing_frame/10.0f, -1.5f-(float)swing_frame/20.0f, 8.0f), sangle, glm::vec3(0.0f, 1.0f, 0.0f));
		smodel = glm::translate(smodel, spos);
		smodel = glm::rotate(smodel, (float) sangle+(float) M_PI/2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		smodel = glm::rotate(smodel, (float) -M_PI/2.0f+(float) (swing_frame/180.0f)*(float)M_PI, glm::vec3(0.0f, 0.0f, 1.0f));

		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(smodel));
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

		swordMesh->render();

		// testing
		testMesh->glBind();

		glm::mat4 testmodel;
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(testmodel));
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

		testMesh->render();

		/* END TESTING */
		// ====================================================================

		// NOTE: swap the front and back buffers
		SDL_GL_SwapWindow(window);
	}

	/* TESTING */
	// ========================================================================

	roomMesh->glUnload();
	testMesh->glUnload();
	swordMesh->glUnload();

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
