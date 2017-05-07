#ifndef __SHADER_PROGRAM_H_
#define __SHADER_PROGRAM_H_

#include "core.h"

//-----------------------------------------------------------------------------
class ShaderProgram {
friend class Mesh;

public:
	typedef enum {
		VERTEX_SHADER,
		FRAGMENT_SHADER,
		NUM_SHADER_TYPES
	} TYPE;

	typedef char Files[NUM_SHADER_TYPES][CORE_MAX_PATH];
	static const int MAX_MESH_PER_SHADER = 50000;
	static const int MAX_SHADER_LOCATIONS = 0x10;
	static const int MAX_SHADER_VAR_LEN = 0x10;

private:
	GLuint _program_index;
	int _next_mesh_ind;
	Files _shader_files;
	bool _has_shader_type[NUM_SHADER_TYPES];
	class Mesh* _meshes[MAX_MESH_PER_SHADER];
	bool _has_attribute[MAX_SHADER_LOCATIONS];
	char _attribute_names[MAX_SHADER_LOCATIONS][MAX_SHADER_VAR_LEN];
	int _attribute_locations[MAX_SHADER_LOCATIONS];
	bool _has_uniform[MAX_SHADER_LOCATIONS];
	char _uniform_names[MAX_SHADER_LOCATIONS][MAX_SHADER_VAR_LEN];
	int _uniform_locations[MAX_SHADER_LOCATIONS];

private:
	bool _load_files(void);
	bool _grab_shaders(GLuint shader_indices[NUM_SHADER_TYPES]);
	void _reload_location(void);
	int _grab_attribute_location(const char* attri_name);
	int _grab_uniform_location(const char* uni_name);

public:
	bool isValid(void);
	bool isInUse(void);
	void use(void);
	void setUniformMatrix4fv(const char* uni_name, const glm::mat4 mat);
	void render(void);
	void addMesh(class Mesh* mesh);

public:
	ShaderProgram(const Files files);
	~ShaderProgram(void);
};

#endif
