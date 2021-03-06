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

	SDL_SetWindowGrab(window, SDL_TRUE);
	SDL_ShowCursor(SDL_DISABLE);

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
	Mesh weapon(program, mesh_data, tex_data);

	mesh_data = ResourceManager::LoadMeshData("data/1_7051_00_0.n3pmesh");
	tex_data = ResourceManager::LoadTextureData("data/item_el_shield_lv2.dxt");
	Mesh shield(program, mesh_data, tex_data);
	//

	//
	SkinData* skin_data = ResourceManager::LoadSkinData("data/st_s_pulreton_3th_2010a01_uio.n3cskins");
	tex_data = ResourceManager::LoadTextureData("data/st_s_pulreton_3th_2010a01_uio.dxt");
	Mesh test_mesh(program, skin_data, tex_data); // new class?

	skin_data = ResourceManager::LoadSkinData("data/st_s_pulreton_3th_2010a01_tyu.n3cskins");
	tex_data = ResourceManager::LoadTextureData("data/st_s_pulreton_3th_2010a01_tyu.dxt");
	Mesh test_mesh2(program, skin_data, tex_data); // new class?

	skin_data = ResourceManager::LoadSkinData("data/st_s_pulreton_3th_2010a01_ser.n3cskins");
	tex_data = ResourceManager::LoadTextureData("data/st_s_pulreton_3th_2010a01_ser.dxt");
	Mesh test_mesh3(program, skin_data, tex_data); // new class?
	//

	//
	skin_data = ResourceManager::LoadSkinData("data/mob_kecoon_foots.n3cskins");
	tex_data = ResourceManager::LoadTextureData("data/npc_mob_devil_foots.dxt");
	Mesh kecoon_feet(program, skin_data, tex_data);

	skin_data = ResourceManager::LoadSkinData("data/mob_kecoon_hands.n3cskins");
	tex_data = ResourceManager::LoadTextureData("data/npc_mob_devil_hands.dxt");
	Mesh kecoon_hands(program, skin_data, tex_data);

	skin_data = ResourceManager::LoadSkinData("data/mob_kecoon_head.n3cskins");
	tex_data = ResourceManager::LoadTextureData("data/npc_mob_devil_face.dxt");
	Mesh kecoon_head(program, skin_data, tex_data);

	skin_data = ResourceManager::LoadSkinData("data/mob_kecoon_lower.n3cskins");
	tex_data = ResourceManager::LoadTextureData("data/npc_mob_devil_lower.dxt");
	Mesh kecoon_lower(program, skin_data, tex_data);

	skin_data = ResourceManager::LoadSkinData("data/mob_kecoon_upper.n3cskins");
	tex_data = ResourceManager::LoadTextureData("data/npc_mob_devil_upper.dxt");
	Mesh kecoon_upper(program, skin_data, tex_data);

	AnimData* anim_data = ResourceManager::LoadAnimData("data/mob_kecoon.n3anim");
	JointData* joint_data = ResourceManager::LoadJointData("data/mob_kecoon.n3joint");
	//

	SDL_Event event = {};
	Uint8* key_state = (Uint8*)SDL_GetKeyboardState(NULL);
	while(event.type!=SDL_QUIT && !key_state[SDL_SCANCODE_ESCAPE]) {
		// ---- UPDATE ----
		while(SDL_PollEvent(&event)) {}
		key_state = (Uint8*)SDL_GetKeyboardState(NULL);

		//
		Uint32 cur_mili;
		static Uint32 prev_mili = SDL_GetTicks();
		cur_mili = SDL_GetTicks();
		float elapsed_secs = ((float)(cur_mili-prev_mili))/1000.0f;
		prev_mili = cur_mili;
		//

		//
		int x, y;
		SDL_GetMouseState(&x, &y);
		int win_w, win_h;
		SDL_GetWindowSize(window, &win_w, &win_h);

		static float m_x = (float)x;
		static float m_y = (float)y;
		m_x = (float)x;
		m_y = (float)y;

		float xoffset = (float)win_w/2 - m_x;
		float yoffset = (float)win_h/2 - m_y;

		float sensitivity = 0.03f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		camera.setYaw(camera.getYaw() + xoffset);
		camera.setPitch(camera.getPitch() + yoffset);

		SDL_WarpMouseInWindow(window, win_w/2, win_h/2);
		//

		//
		glm::vec3 tmp;
		float camera_speed = 10.0f;

		if(key_state[SDL_SCANCODE_A]) {
			tmp = glm::vec3(glm::rotate(glm::mat4(), glm::radians(camera.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f))*glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f));
			camera.setPos(camera.getPos()+tmp*camera_speed*elapsed_secs);
		}
		if(key_state[SDL_SCANCODE_D]) {
			tmp = glm::vec3(glm::rotate(glm::mat4(), glm::radians(camera.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f))*glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));
			camera.setPos(camera.getPos()+tmp*camera_speed*elapsed_secs);
		}
		if(key_state[SDL_SCANCODE_UP]) {
			tmp = glm::vec3(glm::rotate(glm::mat4(), glm::radians(camera.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f))*glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
			camera.setPos(camera.getPos()+tmp*camera_speed*elapsed_secs);
		}
		if(key_state[SDL_SCANCODE_DOWN]) {
			tmp = glm::vec3(glm::rotate(glm::mat4(), glm::radians(camera.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f))*glm::vec4(0.0f, -1.0f, 0.0f, 0.0f));
			camera.setPos(camera.getPos()+tmp*camera_speed*elapsed_secs);
		}
		if(key_state[SDL_SCANCODE_W]) {
			tmp = glm::vec3(glm::rotate(glm::mat4(), glm::radians(camera.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f))*glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
			camera.setPos(camera.getPos()+tmp*camera_speed*elapsed_secs);
		}
		if(key_state[SDL_SCANCODE_S]) {
			tmp = glm::vec3(glm::rotate(glm::mat4(), glm::radians(camera.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f))*glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
			camera.setPos(camera.getPos()+tmp*camera_speed*elapsed_secs);
		}

		camera.update();
		//

		//
		test_mesh.setYaw(test_mesh.getYaw() + 0.75f);
		test_mesh2.setYaw(test_mesh.getYaw() + 0.75f);
		test_mesh3.setYaw(test_mesh.getYaw() + 0.75f);
		//

		//
		kecoon_feet.setPos(glm::vec3(-15.0f, 0.0f, 0.0f));
		kecoon_hands.setPos(glm::vec3(-15.0f, 0.0f, 0.0f));
		kecoon_head.setPos(glm::vec3(-15.0f, 0.0f, 0.0f));
		kecoon_lower.setPos(glm::vec3(-15.0f, 0.0f, 0.0f));
		kecoon_upper.setPos(glm::vec3(-15.0f, 0.0f, 0.0f));
		//

		//
		weapon.setPos(camera.getPos());
		tmp = glm::vec3(-0.15f, -0.55f, -0.6f);
		tmp = glm::rotate(tmp, glm::radians(camera.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f));
		tmp = glm::rotate(tmp, glm::radians(camera.getPitch()), glm::rotate(glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(camera.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f)));
		weapon.translate(tmp);
		weapon.setYaw(camera.getYaw());
		weapon.setRoll(45.0f/2.0f);
		weapon.setPitch(camera.getPitch());

		shield.setPos(camera.getPos());
		tmp = glm::vec3(0.27f, -0.55f, -0.6f);
		tmp = glm::rotate(tmp, glm::radians(camera.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f));
		tmp = glm::rotate(tmp, glm::radians(camera.getPitch()), glm::rotate(glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(camera.getYaw()), glm::vec3(0.0f, 1.0f, 0.0f)));
		shield.translate(tmp);
		shield.setYaw(camera.getYaw()+180.0f);
		//shield.setRoll(45.0f/2.0f);
		shield.setPitch(-camera.getPitch());
		//

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
