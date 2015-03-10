//-----------------------------------------------------------------------------
#ifndef _MESH_H_
#define _MESH_H_

//-----------------------------------------------------------------------------
class Mesh {
private:
	IndInfo* inds;
	VertInfo* verts;
	SDL_Surface* sdl_surf;

	bool glLoaded;
	GLuint vertArray;
	GLuint vertBuffer;
	GLuint textureInd;

protected:
	// transforms?

public:
	Mesh(const char* obj_fn, const char* tex_fn);
	~Mesh(void);

	void render(void);

	void glBind(void);
	void glLoad(void);
	void glUnload(void);
};

//-----------------------------------------------------------------------------
Mesh::Mesh(const char* obj_fn, const char* tex_fn) {
	inds = NULL;
	verts = NULL;
	sdl_surf = NULL;

	loadObjFile(&verts, &inds, obj_fn);

	SDL_Surface* temp_surf = SDL_LoadBMP(tex_fn);

	SDL_PixelFormat nFormat = *temp_surf->format;

	nFormat.Rmask = 0x000000FF;
	nFormat.Gmask = 0x0000FF00;
	nFormat.Bmask = 0x00FF0000;
	nFormat.Amask = 0xFF000000;

	sdl_surf = SDL_ConvertSurface(temp_surf, &nFormat, 0);

	SDL_FreeSurface(temp_surf);
	temp_surf = NULL;

	glLoaded = false;
}

Mesh::~Mesh(void) {
	loadObjFile(&verts, &inds, NULL);

	SDL_FreeSurface(sdl_surf);

	inds = NULL;
	verts = NULL;
	sdl_surf = NULL;
}

void Mesh::render(void) {
	if(!glLoaded) return;

	glDrawArrays(GL_TRIANGLES, 0, inds->num);
}

void Mesh::glBind(void) {
	if(!glLoaded) return;

	glBindVertexArray(vertArray);
	glBindTexture(GL_TEXTURE_2D, textureInd);
}

void Mesh::glLoad(void) {
	// NOTE: generate OpenGL variables
	glGenBuffers(1, &vertBuffer);
	glGenTextures(1, &textureInd);
	glGenVertexArrays(1, &vertArray);

	// NOTE: activate a free texture
	glActiveTexture(getFreeTex());

	// NOTE: bind to the OpenGL variables
	glBindVertexArray(vertArray);
	glBindTexture(GL_TEXTURE_2D, textureInd);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);

	// NOTE: generate the OpenGL vert information
	GLfloat glVerts[5*inds->num];
	memset(glVerts, 0x00, 5*inds->num*sizeof(GLfloat));

	for(int i=0; i<inds->num; i++) {
		glVerts[5*i+0] = verts->pos[3*inds->pos[i]+0];
		glVerts[5*i+1] = verts->pos[3*inds->pos[i]+1];
		glVerts[5*i+2] = verts->pos[3*inds->pos[i]+2];
		glVerts[5*i+3] = verts->tex[2*inds->tex[i]+0];
		glVerts[5*i+4] = verts->tex[2*inds->tex[i]+1];
	}

	// NOTE: pass the information to OpenGL
	glBufferData(GL_ARRAY_BUFFER, 5*inds->num*sizeof(GLfloat), glVerts, GL_STATIC_DRAW);

	// NOTE: pass the texture information to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sdl_surf->w, sdl_surf->h, 0, GL_RGB, GL_UNSIGNED_BYTE, sdl_surf->pixels);
	numLoadedTextures++;

	// NOTE: set the shader program variables
	glUniform1i(glGetUniformLocation(shaderProgram0, "tex"), (numLoadedTextures-1));
	glGenerateMipmap(GL_TEXTURE_2D);

	GLint posAttrib = glGetAttribLocation(shaderProgram0, "pos");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
	glEnableVertexAttribArray(posAttrib);

	GLint texAttrib = glGetAttribLocation(shaderProgram0, "texcoord");
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void *)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(texAttrib);

	glLoaded = true;
}

void Mesh::glUnload(void) {
	glDeleteBuffers(1, &vertBuffer);
	glDeleteTextures(1, &textureInd);
	glDeleteVertexArrays(1, &vertArray);

	glLoaded = false;
}

#endif
