#ifndef __CORE_H_
#define __CORE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <string>

#include "SDL.h"
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/gtx/perpendicular.hpp"
#include "glm/gtc/type_ptr.hpp"

#define CORE_MAX_PATH 260

//-----------------------------------------------------------------------------
struct N3VertexT1: public glm::vec3 {
	glm::vec3 n;
	float tu, tv;
};

struct N3TexHeader {
	char szID[4];
	int nWidth;
	int nHeight;
	int Format;
	bool bMipMap;
};

struct N3EdgeCollapse {
	int NumIndicesToLose;
	int NumIndicesToChange;
	int NumVerticesToLose;
	int iIndexChanges;
	int CollapseTo;
	bool bShouldCollapse;
};

struct N3LODCtrlValue {
	float fDist;
	int iNumVertices;
};

enum D3DFORMAT {
	D3DFMT_UNKNOWN,
	D3DFMT_DXT1 = 827611204,
	D3DFMT_DXT3 = 861165636,
	D3DFMT_DXT5 = 894720068,
	D3DFMT_A1R5G5B5 = 25,
	D3DFMT_A4R4G4B4 = 26,
	D3DFMT_R8G8B8 = 20,
	D3DFMT_A8R8G8B8 = 21,
	D3DFMT_X8R8G8B8 = 22
};

//-----------------------------------------------------------------------------
char* load_file_into_buffer(const char* file_name);

#endif
