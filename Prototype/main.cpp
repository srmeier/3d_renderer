/*
*/

#include "core.h"
#include "mesh.h"
#include "camera.h"
#include "shader_program.h"
#include "resource_manager.h"

//-----------------------------------------------------------------------------
bool is_fullscreen = false;

//-----------------------------------------------------------------------------
int SDL_main(int argc, char** argv) {
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_Log("%s\n", SDL_GetError());
		system("pause");
		exit(-1);
	}

	SDL_Window* window = NULL;
	if(is_fullscreen) {
		window = SDL_CreateWindow(
			"Prototype", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, -1, -1,
			SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL |
			SDL_WINDOW_ALLOW_HIGHDPI
		);
	} else {
		window = SDL_CreateWindow(
			"Prototype", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, 1080, 720, SDL_WINDOW_OPENGL |
			SDL_WINDOW_ALLOW_HIGHDPI
		);
	}

	if(window == NULL) {
		SDL_Log("%s\n", SDL_GetError());
		system("pause");
		exit(-1);
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if(context == NULL) {
		SDL_Log("%s\n", SDL_GetError());
		system("pause");
		exit(-1);
	}

	if(SDL_GL_SetSwapInterval(-1) == -1) {
		if(SDL_GL_SetSwapInterval(1) == -1) {
			SDL_Log("Unable to set swap interval\n");
			system("pause");
			exit(-1);
		}
	}

	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if(glewError != GLEW_OK) {
		SDL_Log("%s\n", glewGetErrorString(glewError));
		system("pause");
		exit(-1);
	}

	SDL_version sdl_version;
	SDL_GetVersion(&sdl_version);
	SDL_Log("SDL (%d.%d.%d)\n", sdl_version.major, sdl_version.minor, sdl_version.patch);
	SDL_Log("OpenGL (%s)\n", glGetString(GL_VERSION));
	SDL_Log("Renderer (%s)\n", glGetString(GL_RENDERER));

	//

	Camera camera(window);
	ShaderProgram* program = camera.getShaderProgram(Camera::TESTING_PROGRAM);

	MeshData* mesh_data = ResourceManager::LoadMeshData("data/1_3561_00_0.n3pmesh");
	TextureData* tex_data = ResourceManager::LoadTextureData("data/doubleaxe_02.dxt");

	Mesh test_mesh(program, mesh_data, tex_data);

	//

	SDL_Event event = {};
	Uint8* key_state = (Uint8*)SDL_GetKeyboardState(NULL);
	while(event.type!=SDL_QUIT && !key_state[SDL_SCANCODE_ESCAPE]) {
		// ---- UPDATE ----
		while(SDL_PollEvent(&event)) {}
		key_state = (Uint8*)SDL_GetKeyboardState(NULL);

		// ---- RENDER ----
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		program->render(); // temp

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
