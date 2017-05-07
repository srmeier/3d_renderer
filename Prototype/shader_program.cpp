/*
*/

#include "mesh.h"
#include "shader_program.h"

//-----------------------------------------------------------------------------
ShaderProgram::ShaderProgram(const Files files) {
	_next_mesh_ind = 0;
	for(int i=0; i<MAX_MESH_PER_SHADER; ++i) {
		_meshes[i] = NULL;
	}

	for(int i=0; i<NUM_SHADER_TYPES; ++i) {
		_has_shader_type[i] = true;
		strcpy(_shader_files[i], files[i]);
		if(!strcmp(files[i], "")) {
			_has_shader_type[i] = false;
			continue;
		}
	}

	if(_load_files() == false) {
		system("pause");
		exit(-1);
	}
}

//-----------------------------------------------------------------------------
ShaderProgram::~ShaderProgram(void) {
	if(isValid()) glDeleteProgram(_program_index);
}

//-----------------------------------------------------------------------------
bool ShaderProgram::_load_files(void) {
	if(isValid()) glDeleteProgram(_program_index);

	GLuint shader_indices[NUM_SHADER_TYPES] = {};
	if(_grab_shaders(shader_indices) == false) return false;

	_program_index = glCreateProgram();
	for(int i=0; i<NUM_SHADER_TYPES; ++i) {
		if(_has_shader_type[i]) glAttachShader(_program_index, shader_indices[i]);
	}

	glLinkProgram(_program_index);
	for(int i=0; i<NUM_SHADER_TYPES; ++i) {
		if(_has_shader_type[i]) {
			glDetachShader(_program_index, shader_indices[i]);
			glDeleteShader(shader_indices[i]);
		}
	}

	int params;
	glGetProgramiv(_program_index, GL_LINK_STATUS, &params);
	if(params != GL_TRUE) {
		SDL_Log("Count not link shader program \"%d\"\n", _program_index);
		glDeleteProgram(_program_index);
		return false;
	}

	_reload_location();
	return isValid();
}

//-----------------------------------------------------------------------------
bool ShaderProgram::_grab_shaders(GLuint shader_indices[NUM_SHADER_TYPES]) {
	for(int i=0; i<NUM_SHADER_TYPES; ++i) {
		if(!_has_shader_type[i]) continue;

		switch(i) {
			case VERTEX_SHADER:
				shader_indices[i] = glCreateShader(GL_VERTEX_SHADER);
				break;
			case FRAGMENT_SHADER:
				shader_indices[i] = glCreateShader(GL_FRAGMENT_SHADER);
				break;
			default: return false;
		}

		char* shader_source = load_file_into_buffer(_shader_files[i]);
		if(shader_source == NULL) return false;

		glShaderSource(shader_indices[i], 1, &shader_source, NULL);
		glCompileShader(shader_indices[i]);

		free(shader_source);

		int params;
		glGetShaderiv(shader_indices[i], GL_COMPILE_STATUS, &params);
		if(params != GL_TRUE) {
			SDL_Log("Failed to compile shader \"%d\"\n", shader_indices[i]);
			glDeleteShader(shader_indices[i]);
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
void ShaderProgram::_reload_location(void) {
	int params = -1;
	memset(_has_attribute, 0x00, MAX_SHADER_LOCATIONS*sizeof(bool));
	glGetProgramiv(_program_index, GL_ACTIVE_ATTRIBUTES, &params);
	for(GLuint i=0; i<(GLuint)params; ++i) {
		if(i >= MAX_SHADER_LOCATIONS) {
			SDL_Log("Too many attributes in the shader\n");
			continue;
		}

		GLenum type;
		int size = 0;
		int max_length = MAX_SHADER_VAR_LEN;
		int actual_length = 0;

		glGetActiveAttrib(
			_program_index, i, max_length, &actual_length,
			&size, &type, _attribute_names[i]
		);

		if(size > 1) {
			SDL_Log("Need to implement this (size>1)\n");
		} else {
			_has_attribute[i] = true;
			_attribute_locations[i] = glGetAttribLocation(
				_program_index, _attribute_names[i]
			);
		}
	}

	memset(_has_uniform, 0x00, MAX_SHADER_LOCATIONS*sizeof(bool));
	glGetProgramiv(_program_index, GL_ACTIVE_UNIFORMS, &params);
	for(GLuint i=0; i<(GLuint)params; ++i) {
		if(i >= MAX_SHADER_LOCATIONS) {
			SDL_Log("Too many attributes in the shader\n");
			continue;
		}

		GLenum type;
		int size = 0;
		int max_length = MAX_SHADER_VAR_LEN;
		int actual_length = 0;

		glGetActiveUniform(
			_program_index, i, max_length, &actual_length,
			&size, &type, _uniform_names[i]
		);

		if(size > 1) {
			SDL_Log("Need to implement this (size>1)\n");
		} else {
			_has_uniform[i] = true;
			_uniform_locations[i] = glGetUniformLocation(
				_program_index, _uniform_names[i]
			);
		}
	}

	use();

	setUniformMatrix4fv("model", glm::mat4());
	setUniformMatrix4fv("view" , glm::mat4());
	setUniformMatrix4fv("proj" , glm::mat4());
}

//-----------------------------------------------------------------------------
int ShaderProgram::_grab_attribute_location(const char* attri_name) {
	for(int i=0; i<MAX_SHADER_LOCATIONS; ++i) {
		if(!_has_attribute[i]) continue;
		if(!strcmp(_attribute_names[i], attri_name)) {
			return _attribute_locations[i];
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------
int ShaderProgram::_grab_uniform_location(const char* uni_name) {
	for(int i=0; i<MAX_SHADER_LOCATIONS; ++i) {
		if(!_has_uniform[i]) continue;
		if(!strcmp(_uniform_names[i], uni_name)) {
			return _uniform_locations[i];
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------
bool ShaderProgram::isValid(void) {
	int is_valid;
	glValidateProgram(_program_index);
	glGetProgramiv(_program_index, GL_VALIDATE_STATUS, &is_valid);
	return (is_valid == GL_TRUE);
}

//-----------------------------------------------------------------------------
bool ShaderProgram::isInUse(void) {
	int active_index;
	glGetIntegerv(GL_CURRENT_PROGRAM, &active_index);
	return (_program_index == active_index);
}

//-----------------------------------------------------------------------------
void ShaderProgram::use(void) {
	if(!isInUse()) glUseProgram(_program_index);
}

//-----------------------------------------------------------------------------
void ShaderProgram::setUniformMatrix4fv(const char* uni_name, const glm::mat4 mat) {
	if(!isInUse()) {
		SDL_Log("Program is not set for uniform \"%s\"\n", uni_name);
		return;
	}

	int location = _grab_uniform_location(uni_name);
	if(location == -1) {
		SDL_Log("The program doesn't have a uniform named \"%s\"\n", uni_name);
		return;
	}

	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

//-----------------------------------------------------------------------------
void ShaderProgram::render(void) {
	for(int i=0; i<MAX_MESH_PER_SHADER; ++i) {
		if(_meshes[i]) _meshes[i]->render(this);
	}
}

//-----------------------------------------------------------------------------
void ShaderProgram::addMesh(Mesh* mesh) {
	if(_next_mesh_ind < MAX_MESH_PER_SHADER) {
		_meshes[_next_mesh_ind] = mesh;
		while(_meshes[_next_mesh_ind] && _next_mesh_ind<MAX_MESH_PER_SHADER)
			_next_mesh_ind++;
	} else {
		SDL_Log("Mesh count is full for this shader program\n");
	}
}
