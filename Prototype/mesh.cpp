/*
*/

#include "mesh.h"
#include "shader_program.h"

//-----------------------------------------------------------------------------
Mesh::Mesh(ShaderProgram* program, MeshData* mesh_data, TextureData* texture_data) {
	_mesh_data = mesh_data;
	_texture_data = texture_data;
	_skin_data = NULL;
	_pos = glm::vec3(0.f, 0.0f, 2.f);
	_yaw = 0.0f;
	_pitch = 0.0f;
	_roll = 0.0f;

	program->addMesh(this);

	glGenVertexArrays(1, &_vert_array);
	glGenBuffers(1, &_vert_buffer);
	glGenBuffers(1, &_elem_buffer);
	glGenTextures(1, &_tex_buffer);

	glBindVertexArray(_vert_array);

	GLfloat* vertices = new GLfloat[5*mesh_data->num_verts];
	memset(vertices, 0, 5*mesh_data->num_verts*sizeof(GLfloat));

	for(int i=0; i<mesh_data->num_verts; i++) {
		vertices[5*i+0] = mesh_data->verts[i].x;
		vertices[5*i+1] = mesh_data->verts[i].y;
		vertices[5*i+2] = mesh_data->verts[i].z;

		vertices[5*i+3] = mesh_data->verts[i].tu;
		vertices[5*i+4] = mesh_data->verts[i].tv;
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vert_buffer);
	glBufferData(GL_ARRAY_BUFFER, 5*mesh_data->num_verts*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	delete vertices;

	int attri_pos_loc = program->_grab_attribute_location("vert_pos");
	glVertexAttribPointer(attri_pos_loc, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
	glEnableVertexAttribArray(attri_pos_loc);
	int attri_tex_loc = program->_grab_attribute_location("vt_coords");
	glVertexAttribPointer(attri_tex_loc, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(attri_tex_loc);

	GLuint* elements = new GLuint[mesh_data->num_indices];
	memset(elements, 0, mesh_data->num_indices*sizeof(GLuint));
	for(int i=0; i<mesh_data->num_indices; i++) {
		elements[i] = (GLuint) mesh_data->indices[i];
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elem_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_data->num_indices*sizeof(GLuint), elements, GL_STATIC_DRAW);
	delete elements;

	_set_texture(program, texture_data);
}

//-----------------------------------------------------------------------------
Mesh::Mesh(ShaderProgram* program, SkinData* skin_data, TextureData* texture_data) {
	_mesh_data = NULL;
	_texture_data = texture_data;
	_skin_data = skin_data;
	_pos = glm::vec3(0.f, 0.0f, 2.f);
	_yaw = 0.0f;
	_pitch = 0.0f;
	_roll = 0.0f;

	program->addMesh(this);

	glGenVertexArrays(1, &_vert_array);
	glGenBuffers(1, &_vert_buffer);
	glGenBuffers(1, &_elem_buffer);
	glGenTextures(1, &_tex_buffer);

	glBindVertexArray(_vert_array);

	GLfloat* vertices = new GLfloat[5*3*skin_data->face_count];
	memset(vertices, 0, 5*3*skin_data->face_count);

	for(int i=0; i<3*skin_data->face_count; i++) {
		vertices[5*i+0] = skin_data->verts[skin_data->indices[i]].x;
		vertices[5*i+1] = skin_data->verts[skin_data->indices[i]].y;
		vertices[5*i+2] = skin_data->verts[skin_data->indices[i]].z;

		vertices[5*i+3] = skin_data->uvs[2*skin_data->uv_indices[i]+0];
		vertices[5*i+4] = skin_data->uvs[2*skin_data->uv_indices[i]+1];
	}

	glBindBuffer(GL_ARRAY_BUFFER, _vert_buffer);
	glBufferData(GL_ARRAY_BUFFER, 5*3*skin_data->face_count*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	delete vertices;

	int attri_pos_loc = program->_grab_attribute_location("vert_pos");
	glVertexAttribPointer(attri_pos_loc, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
	glEnableVertexAttribArray(attri_pos_loc);
	int attri_tex_loc = program->_grab_attribute_location("vt_coords");
	glVertexAttribPointer(attri_tex_loc, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(attri_tex_loc);

	GLuint* elements = new GLuint[3*skin_data->face_count];
	memset(elements, 0, 3*skin_data->face_count*sizeof(GLuint));
	for(int i=0; i<3*skin_data->face_count; i++) {
		elements[i] = (GLuint) (i);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _elem_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*skin_data->face_count*sizeof(GLuint), elements, GL_STATIC_DRAW);
	delete elements;

	_set_texture(program, texture_data);
}

//-----------------------------------------------------------------------------
Mesh::~Mesh(void) {
	// TODO: release the GL stuff
}

//-----------------------------------------------------------------------------
void Mesh::_set_texture(ShaderProgram* program, TextureData* texture_data) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _tex_buffer);
	
	GLenum tex_type;
	GLenum tex_format;
	switch(texture_data->header.Format) {
		case D3DFMT_DXT1: {
			tex_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		} break;
		case D3DFMT_DXT3: {
			tex_format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		} break;
		case D3DFMT_DXT5: {
			tex_format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		} break;
		case D3DFMT_A1R5G5B5: {
			tex_format = GL_RGBA;
			tex_type = GL_UNSIGNED_SHORT_5_5_5_1;
			for(int i=0; i<texture_data->header.nWidth*texture_data->header.nHeight; ++i) {
				unsigned short* pp = (unsigned short*)(texture_data->pixels + texture_data->size*i);
				unsigned short p = *pp;
				unsigned short np = ((p&0x7C00)>>10)<<11|((p&0x3E0)>>5)<<6|(p&0x1F)<<1|((p&0x8000)>>15);
				*pp = np;
			}
		} break;
		case D3DFMT_A4R4G4B4: {
			tex_format = GL_RGBA;
			tex_type = GL_UNSIGNED_SHORT_4_4_4_4;
			for(int i=0; i<texture_data->header.nWidth*texture_data->header.nHeight; ++i) {
				unsigned short* pp = (unsigned short*)(texture_data->pixels + texture_data->size*i);
				unsigned short p = *pp;
				unsigned short np = ((p&0xF00)>>8)<<12|((p&0xF0)>>4)<<8|(p&0xF)<<4|((p&0xF000)>>12);
				*pp = np;
			}
		} break;
		case D3DFMT_R8G8B8: {
			tex_format = GL_RGB;
			tex_type = GL_UNSIGNED_BYTE;
			SDL_Log("Need to implement this D3DFMT_R8G8B8\n");
			return;
		} break;
		case D3DFMT_A8R8G8B8: {
			tex_format = GL_RGBA;
			tex_type = GL_UNSIGNED_INT_8_8_8_8;
			SDL_Log("Need to implement this D3DFMT_A8R8G8B8\n");
			return;
		} break;
		case D3DFMT_X8R8G8B8: {
			tex_format = GL_RGBA;
			tex_type = GL_UNSIGNED_INT_8_8_8_8;
			SDL_Log("Need to implement this D3DFMT_X8R8G8B8\n");
			return;
		} break;
		default: {
			SDL_Log("Unknown texture format %d\n", texture_data->header.Format);
			return;
		} break;
	}

	if(texture_data->is_compressed) {
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, tex_format, texture_data->header.nWidth, texture_data->header.nHeight, 0, texture_data->size, texture_data->pixels);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, tex_format, texture_data->header.nWidth, texture_data->header.nHeight, 0, tex_format, tex_type, texture_data->pixels);
	}

	glUniform1i(program->_grab_uniform_location("basic_texture"), 0);
	glGenerateMipmapEXT(GL_TEXTURE_2D);
}

//-----------------------------------------------------------------------------
void Mesh::render(ShaderProgram* program) {
	// TODO: add a "dirty" check
	glm::mat4 model = glm::translate(glm::mat4(), _pos);
	model = glm::rotate(model, glm::radians(_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(_pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(_roll), glm::vec3(0.0f, 0.0f, 1.0f));

	program->use();
	program->setUniformMatrix4fv("model", model); // temp?

	glBindVertexArray(_vert_array);

	glActiveTexture(GL_TEXTURE0); // temp?
	glBindTexture(GL_TEXTURE_2D, _tex_buffer); // temp?

	if(_mesh_data) {
		glDrawElements(GL_TRIANGLES, _mesh_data->num_indices, GL_UNSIGNED_INT, 0);
	} else {
		glDrawElements(GL_TRIANGLES, 3*_skin_data->face_count, GL_UNSIGNED_INT, 0);
	}
}
