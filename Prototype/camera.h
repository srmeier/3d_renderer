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
	float _yaw, _pitch, _roll;

public:
	void setView(void);
	void setProjection(void);
	class ShaderProgram* getShaderProgram(SHADER_PROGRAM_TYPE type) {
		return _shader_programs[type];
	}
	glm::vec3 getPos(void) {return _pos;};
	void setPos(glm::vec3 pos) {_pos = pos;}
	float getYaw(void) {return _yaw;}
	float getPitch(void) {return _pitch;}
	void setYaw(float yaw) {_yaw = yaw;}
	void setPitch(float pitch) {_pitch = pitch;}
	void setRoll(float roll) {_roll = roll;}
	float getRoll(void) {return _roll;}
	void update(void);

public:
	Camera(SDL_Window* window);
	~Camera(void);
};

#endif
