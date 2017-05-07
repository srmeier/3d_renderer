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

public:
	void render(class ShaderProgram* program);

public:
	Mesh(class ShaderProgram* program, MeshData* mesh_data, TextureData* texture_data);
	~Mesh(void);
};

#endif
