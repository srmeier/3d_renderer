#ifndef __MESH_H_
#define __MESH_H_

#include "core.h"
#include "resource_manager.h"

//-----------------------------------------------------------------------------
class Mesh {
private:
	GLuint _vert_array;
	GLuint _vert_buffer;
	GLuint _elem_buffer;
	GLuint _tex_buffer;
	MeshData* _mesh_data;
	TextureData* _texture_data;
	SkinData* _skin_data;
	glm::vec3 _pos;
	float _yaw, _pitch, _roll;

private:
	void _set_texture(class ShaderProgram* program, TextureData* texture_data);

public:
	void render(class ShaderProgram* program);
	glm::vec3 getPos(void) {return _pos;};
	void setPos(glm::vec3 pos) {_pos = pos;}
	float getYaw(void) {return _yaw;}
	float getPitch(void) {return _pitch;}
	void setYaw(float yaw) {_yaw = yaw;}
	void setPitch(float pitch) {_pitch = pitch;}
	void setRoll(float roll) {_roll = roll;}
	float getRoll(void) {return _roll;}
	void translate(glm::vec3 vec) {_pos += vec;}

public:
	Mesh(class ShaderProgram* program, MeshData* mesh_data, TextureData* texture_data);
	Mesh(class ShaderProgram* program, SkinData* skin_data, TextureData* texture_data);
	~Mesh(void);
};

#endif
