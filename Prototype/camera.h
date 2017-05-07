#ifndef __CAMERA_H_
#define __CAMERA_H_

#include "core.h"

//-----------------------------------------------------------------------------
class Camera {
public:
	typedef enum {
		TESTING_PROGRAM,
		NUM_SHADER_PROGRAM_TYPES
	} SHADER_PROGRAM_TYPE;

private:
	SDL_Window* _window;
	class ShaderProgram* _shader_programs[NUM_SHADER_PROGRAM_TYPES];
	int _framebuffer_w, _framebuffer_h;
	glm::vec3 _pos;
	float _yaw, _pitch;

public:
	void setView(void);
	void setProjection(void);
	class ShaderProgram* getShaderProgram(SHADER_PROGRAM_TYPE type) {
		return _shader_programs[type];
	}

public:
	Camera(SDL_Window* window);
	~Camera(void);
};

#endif
