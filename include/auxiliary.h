//-----------------------------------------------------------------------------
#ifndef _AUXILIARY_H_
#define _AUXILIARY_H_

//-----------------------------------------------------------------------------
size_t numLoadedTextures = 0;

//-----------------------------------------------------------------------------
GLenum getFreeTex(void) {
	switch(numLoadedTextures) {
		case 0x00: return GL_TEXTURE0;
		case 0x01: return GL_TEXTURE1;
		case 0x02: return GL_TEXTURE2;
		case 0x03: return GL_TEXTURE3;
		case 0x04: return GL_TEXTURE4;
		case 0x05: return GL_TEXTURE5;
		case 0x06: return GL_TEXTURE6;
		case 0x07: return GL_TEXTURE7;
		default: return GL_TEXTURE8;
	}
}

//-----------------------------------------------------------------------------
typedef struct {
	GLfloat* pos;   // (x,y,z)
	GLfloat* tex;   // (u,v)
	GLfloat* norm;  // (x,y,z)
	GLsizeiptr num; // total num of verts
} VertInfo;

typedef struct {
	GLuint* pos;    // 3 pos inds per
	GLuint* tex;    // 3 tex inds per
	GLuint* norm;   // 3 norm inds per
	GLsizeiptr num; // num of triangles
} IndInfo;

//-----------------------------------------------------------------------------
void loadObjFile(VertInfo** verts, IndInfo** inds, const char* filename) {
	if(*verts) {
		free((*verts)->pos);
		(*verts)->pos = NULL;
		free((*verts)->tex);
		(*verts)->tex = NULL;
		free((*verts)->norm);
		(*verts)->norm = NULL;

		(*verts)->num = 0;
	} else *verts = (VertInfo*) calloc(0x01, sizeof(VertInfo));

	if(*inds) {
		free((*inds)->pos);
		(*inds)->pos = NULL;
		free((*inds)->tex);
		(*inds)->tex = NULL;
		free((*inds)->norm);
		(*inds)->norm = NULL;
		
		(*inds)->num = 0;
	} else *inds = (IndInfo*) calloc(0x01, sizeof(IndInfo));

	size_t n = 0;
	char** lines = NULL;

	if(filename==NULL) return;
	FILE* fp = fopen(filename, "r");

	do {
		char c;
		int colInd = 0;

		lines = (char**) realloc(lines, ++n*sizeof(char*));
		lines[n-1] = NULL;

		do {
			fread(&c, sizeof(char), 1, fp);

			// NOTE: strip newlines
			if(c!='\n') {
				if(colInd>0) {
					// NOTE: strip extra spaces
					if(!(lines[n-1][colInd-1]==' ' && c==' ')) {
						lines[n-1] = (char*) realloc(lines[n-1], ++colInd*sizeof(char));
						lines[n-1][colInd-1] = c;
					}
				} else {
					lines[n-1] = (char*) realloc(lines[n-1], ++colInd*sizeof(char));
					lines[n-1][colInd-1] = c;
				}
			}
		} while(c!='\n');

		// NOTE: end with a null terminator
		lines[n-1] = (char*) realloc(lines[n-1], ++colInd*sizeof(char));
		lines[n-1][colInd-1] = '\0';

		if(lines[n-1][0]=='#') {
			// NOTE: ignore comments

			free(lines[n-1]);
			lines[n-1] = NULL;

			lines = (char**) realloc(lines, --n*sizeof(char*));
		} else if(lines[n-1][0]=='\0') {
			// NOTE: ignore blank lines

			free(lines[n-1]);
			lines[n-1] = NULL;

			lines = (char**) realloc(lines, --n*sizeof(char*));
		}
	} while(!feof(fp));

	fclose(fp);
	fp = NULL;

	size_t numPos = 0;
	size_t numTex = 0;

	int i;
	for(i=0; i<n; i++) {
		// NOTE: parse the file
		if(lines[i][0]=='v' && lines[i][1]==' ') {
			// NOTE: parse the vertex and add it to the array

			(*verts)->num++;

			numPos += 3;
			(*verts)->pos = (GLfloat*) realloc((*verts)->pos, numPos*sizeof(GLfloat));

			GLfloat x = 0, y = 0, z = 0;
			sscanf(lines[i], "v %f %f %f", &x, &y, &z);

			(*verts)->pos[numPos-3] = x;
			(*verts)->pos[numPos-2] = y;
			(*verts)->pos[numPos-1] = z;

		} else if(lines[i][0]=='v' && lines[i][1]=='t' && lines[i][2]==' ') {
			// NOTE: parse the texture coordinates

			numTex += 2;
			(*verts)->tex = (GLfloat*) realloc((*verts)->tex, numTex*sizeof(GLfloat));

			GLfloat u = 0, v = 0;
			sscanf(lines[i], "vt %f %f", &u, &v);

			(*verts)->tex[numTex-2] = u;
			(*verts)->tex[numTex-1] = v;
			
		} else if(lines[i][0] == 'f' && lines[i][1] == ' ') {
			// NOTE: parse the face elements

			int c, num = 0;
			GLuint* indices = NULL;

			for(c=1; c<strlen(lines[i]); c++) {

				// TODO: will have to handle the case when normals are left out
				// or when texture indices are left out

				// TODO: currently I am assuming 4 indices per face which makes
				// two triangles (i.e. 6 elements)

				if(lines[i][c]==' ') {
					num += 3;
					indices = (GLuint*) realloc(indices, num*sizeof(GLuint));

					GLuint vi = 0, vti = 0, vni = 0;
					sscanf(&lines[i][c], " %d/%d/%d", &vi, &vti, &vni);

					indices[num-3] = vi;
					indices[num-2] = vti;
					indices[num-1] = vni;
				}
			}

			if(num==3*3) {
				// NOTE: triangle

				// TODO: currently only setting the position index but I need
				// to set them all eventually

				(*inds)->num += 3;

				(*inds)->pos = (GLuint*) realloc((*inds)->pos, (*inds)->num*sizeof(GLuint));
				(*inds)->tex = (GLuint*) realloc((*inds)->tex, (*inds)->num*sizeof(GLuint));
				(*inds)->norm = (GLuint*) realloc((*inds)->norm, (*inds)->num*sizeof(GLuint));

				int pos0 = indices[0];
				int pos1 = indices[3];
				int pos2 = indices[6];

				(*inds)->pos[(*inds)->num-3] = pos0-1;
				(*inds)->pos[(*inds)->num-2] = pos1-1;
				(*inds)->pos[(*inds)->num-1] = pos2-1;

				int tex0 = indices[1];
				int tex1 = indices[4];
				int tex2 = indices[7];

				(*inds)->tex[(*inds)->num-3] = tex0-1;
				(*inds)->tex[(*inds)->num-2] = tex1-1;
				(*inds)->tex[(*inds)->num-1] = tex2-1;

				int norm0 = indices[2];
				int norm1 = indices[5];
				int norm2 = indices[8];

				(*inds)->norm[(*inds)->num-3] = norm0-1;
				(*inds)->norm[(*inds)->num-2] = norm1-1;
				(*inds)->norm[(*inds)->num-1] = norm2-1;

			} else if(num==4*3) {
				// NOTE: quadrilateral

				// TODO: currently only setting the position index but I need
				// to set them all eventually

				(*inds)->num += 6;

				(*inds)->pos = (GLuint*) realloc((*inds)->pos, (*inds)->num*sizeof(GLuint));
				(*inds)->tex = (GLuint*) realloc((*inds)->tex, (*inds)->num*sizeof(GLuint));
				(*inds)->norm = (GLuint*) realloc((*inds)->norm, (*inds)->num*sizeof(GLuint));

				int pos0 = indices[0];
				int pos1 = indices[3];
				int pos2 = indices[6];
				int pos3 = indices[9];

				(*inds)->pos[(*inds)->num-6] = pos0-1;
				(*inds)->pos[(*inds)->num-5] = pos1-1;
				(*inds)->pos[(*inds)->num-4] = pos2-1;
				(*inds)->pos[(*inds)->num-3] = pos2-1;
				(*inds)->pos[(*inds)->num-2] = pos3-1;
				(*inds)->pos[(*inds)->num-1] = pos0-1;

				int tex0 = indices[1];
				int tex1 = indices[4];
				int tex2 = indices[7];
				int tex3 = indices[10];

				(*inds)->tex[(*inds)->num-6] = tex0-1;
				(*inds)->tex[(*inds)->num-5] = tex1-1;
				(*inds)->tex[(*inds)->num-4] = tex2-1;
				(*inds)->tex[(*inds)->num-3] = tex2-1;
				(*inds)->tex[(*inds)->num-2] = tex3-1;
				(*inds)->tex[(*inds)->num-1] = tex0-1;

				int norm0 = indices[2];
				int norm1 = indices[5];
				int norm2 = indices[8];
				int norm3 = indices[11];

				(*inds)->norm[(*inds)->num-6] = norm0-1;
				(*inds)->norm[(*inds)->num-5] = norm1-1;
				(*inds)->norm[(*inds)->num-4] = norm2-1;
				(*inds)->norm[(*inds)->num-3] = norm2-1;
				(*inds)->norm[(*inds)->num-2] = norm3-1;
				(*inds)->norm[(*inds)->num-1] = norm0-1;
			}

			free(indices);
			indices = NULL;
		}
	}

	for(i=0; i<n; i++) {
		free(lines[i]);
		lines[i] = NULL;
	}

	free(lines);
	lines = NULL;
}

#endif
