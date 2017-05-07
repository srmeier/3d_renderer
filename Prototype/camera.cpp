/*
*/

#include "camera.h"
#include "shader_program.h"

//-----------------------------------------------------------------------------
Camera::Camera(SDL_Window* window) {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	_window = window;
	_framebuffer_w = 0;
	_framebuffer_h = 0;
	_pos = glm::vec3(0.f, 0.5f, 4.f);
	_yaw = 0.0f;
	_pitch = 0.0f;
	_roll = 0.0f;

	ShaderProgram::Files shader_files[] = {
		{"test.vert", "test.frag"}
	};

	for(int i=0; i<NUM_SHADER_PROGRAM_TYPES; ++i) {
		_shader_programs[i] = new ShaderProgram(shader_files[i]);
	}

	_shader_programs[TESTING_PROGRAM]->use();

	setView();
	setProjection();
}

//-----------------------------------------------------------------------------
Camera::~Camera(void) {
	for(int i=0; i<NUM_SHADER_PROGRAM_TYPES; ++i) {
		delete _shader_programs[i];
	}
}

//-----------------------------------------------------------------------------
void Camera::setView(void) {
	// TODO: add a "dirty" check
	glm::mat4 view = glm::translate(glm::mat4(), _pos);
	view = glm::rotate(view, glm::radians(_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
	view = glm::rotate(view, glm::radians(_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	view = glm::rotate(view, glm::radians(_roll), glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::inverse(view);

	for(int i=0; i<NUM_SHADER_PROGRAM_TYPES; ++i) {
		_shader_programs[i]->use();
		_shader_programs[i]->setUniformMatrix4fv("view", view);
	}
}

//-----------------------------------------------------------------------------
void Camera::setProjection(void) {
	SDL_GL_GetDrawableSize(_window, &_framebuffer_w, &_framebuffer_h);
	glViewport(0, 0, _framebuffer_w, _framebuffer_h);

	glm::mat4 proj;
	proj = glm::perspective(45.0f,
		(float)_framebuffer_w/(float)_framebuffer_h, 0.1f, 512.0f
	);

	for(int i=0; i<NUM_SHADER_PROGRAM_TYPES; ++i) {
		_shader_programs[i]->use();
		_shader_programs[i]->setUniformMatrix4fv("proj", proj);
	}
}

//-----------------------------------------------------------------------------
void Camera::update(void) {
	setView();
}
