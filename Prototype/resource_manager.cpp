/*
*/

#include "core.h"
#include "resource_manager.h"

//-----------------------------------------------------------------------------
std::map<std::string, MeshData> ResourceManager::_mesh_data_map;
std::map<std::string, TextureData> ResourceManager::_texture_data_map;
std::map<std::string, SkinData> ResourceManager::_skin_data_map;
std::map<std::string, AnimData> ResourceManager::_anim_data_map;
std::map<std::string, JointData> ResourceManager::_joint_data_map;

//-----------------------------------------------------------------------------
MeshData* ResourceManager::LoadMeshData(const std::string filename) {
	std::map<std::string, MeshData>::iterator iter;

	iter = _mesh_data_map.find(filename);
	if(iter != _mesh_data_map.end()) {
		return &iter->second;
	}

	MeshData mesh_data = {};
	FILE* file_ptr = fopen(filename.c_str(), "rb");
	if(file_ptr == NULL) {
		SDL_Log("Missing mesh \"%s\"\n", filename.c_str());
		return NULL;
	}

	int name_length = 0;
	fread(&name_length, sizeof(int), 1, file_ptr);

	char name[0xFF] = {};
	if(name_length > 0) {
		memset(name, 0, (name_length+1));
		fread(name, sizeof(char), name_length, file_ptr);
	}

	int num_collapses;
	int total_index_changes;
	int min_num_vertices;
	int min_num_indices;

	fread(&num_collapses, sizeof(int), 1, file_ptr);
	fread(&total_index_changes, sizeof(int), 1, file_ptr);
	fread(&mesh_data.num_verts, sizeof(int), 1, file_ptr);
	fread(&mesh_data.num_indices, sizeof(int), 1, file_ptr);
	fread(&min_num_vertices, sizeof(int), 1, file_ptr);
	fread(&min_num_indices, sizeof(int), 1, file_ptr);

	if(mesh_data.num_verts > 0) {
		mesh_data.verts = new N3VertexT1[mesh_data.num_verts];
		memset(mesh_data.verts, 0, sizeof(N3VertexT1)*mesh_data.num_verts);
		fread(mesh_data.verts, sizeof(N3VertexT1), mesh_data.num_verts, file_ptr);
	}

	if(mesh_data.num_indices > 0) {
		mesh_data.indices = new unsigned short[mesh_data.num_indices];
		memset(mesh_data.indices, 0, sizeof(unsigned short)*mesh_data.num_indices);
		fread(mesh_data.indices, sizeof(unsigned short), mesh_data.num_indices, file_ptr);
	}

	N3EdgeCollapse* collapses = new N3EdgeCollapse[(num_collapses+1)];
	memset(&collapses[num_collapses], 0, sizeof(N3EdgeCollapse));

	if(num_collapses > 0) {
		fread(collapses, sizeof(N3EdgeCollapse), num_collapses, file_ptr);
		memset(&collapses[num_collapses], 0, sizeof(N3EdgeCollapse));
	}

	int* all_index_changes = NULL;
	if(total_index_changes > 0) {
		all_index_changes = new int[total_index_changes];
		fread(all_index_changes, sizeof(int), total_index_changes, file_ptr);
	}

	int lod_ctrl_values_count;
	fread(&lod_ctrl_values_count, sizeof(int), 1, file_ptr);

	N3LODCtrlValue* lod_ctrl_values = NULL;
	if(lod_ctrl_values_count) {
		lod_ctrl_values = new N3LODCtrlValue[lod_ctrl_values_count];
		fread(lod_ctrl_values, sizeof(N3LODCtrlValue), lod_ctrl_values_count, file_ptr);
	}

	if(all_index_changes && lod_ctrl_values_count) {
		int c = 0;
		int LOD = 0;
		int m_iNumIndices = 0;
		int m_iNumVertices = 0;

		// TEMP HACK
		//if(m_szName0[0] == 'a') {
		//	LOD = 1;
		//}

		int iDiff = lod_ctrl_values[LOD].iNumVertices - m_iNumVertices;

		while(lod_ctrl_values[LOD].iNumVertices > m_iNumVertices) {
			if(c >= num_collapses) break;
			if(collapses[c].NumVerticesToLose+m_iNumVertices > lod_ctrl_values[LOD].iNumVertices)
				break;

			m_iNumIndices += collapses[c].NumIndicesToLose;
			m_iNumVertices += collapses[c].NumVerticesToLose;
		
			int tmp0 = collapses[c].iIndexChanges;
			int tmp1 = tmp0+collapses[c].NumIndicesToChange;

			for(int i=tmp0; i<tmp1; i++) {
				mesh_data.indices[all_index_changes[i]] = m_iNumVertices-1;
			}

			c++;

			mesh_data.num_indices = m_iNumIndices;
			mesh_data.num_verts = m_iNumVertices;
		}

		while(collapses[c].bShouldCollapse) {

			if(c >= num_collapses) break;

			m_iNumIndices += collapses[c].NumIndicesToLose;
			m_iNumVertices += collapses[c].NumVerticesToLose;
		
			int tmp0 = collapses[c].iIndexChanges;
			int tmp1 = tmp0+collapses[c].NumIndicesToChange;

			for(int i=tmp0; i<tmp1; i++) {
				mesh_data.indices[all_index_changes[i]] = m_iNumVertices-1;
			}

			c++;

			mesh_data.num_indices = m_iNumIndices;
			mesh_data.num_verts = m_iNumVertices;
		}
	}

	free(lod_ctrl_values);
	free(all_index_changes);
	free(collapses);

	fclose(file_ptr);
	
	SDL_Log("------------------------------------------------------------");
	SDL_Log("MeshName: %s\n", name);
	SDL_Log("m_iNumCollapses      -> %d\n", num_collapses);
	SDL_Log("m_iTotalIndexChanges -> %d\n", total_index_changes);
	SDL_Log("m_iMaxNumVertices    -> %d\n", mesh_data.num_verts);
	SDL_Log("m_iMaxNumIndices     -> %d\n", mesh_data.num_indices);
	SDL_Log("m_iMinNumVertices    -> %d\n", min_num_vertices);
	SDL_Log("m_iMinNumIndices     -> %d\n", min_num_indices);
	SDL_Log("m_iLODCtrlValueCount -> %d\n", lod_ctrl_values_count);

	std::pair<std::map<std::string, MeshData>::iterator, bool> ret;
	ret = _mesh_data_map.insert(std::make_pair(filename, mesh_data));
	if(ret.second == false) {
		SDL_Log("Duplicate!!\n");
		exit(-1);
	}

	return &ret.first->second;
}

//-----------------------------------------------------------------------------
TextureData* ResourceManager::LoadTextureData(const std::string filename) {
	std::map<std::string, TextureData>::iterator iter;

	iter = _texture_data_map.find(filename);
	if(iter != _texture_data_map.end()) {
		return &iter->second;
	}

	TextureData texture_data = {};
	FILE* file_ptr = fopen(filename.c_str(), "rb");
	if(file_ptr == NULL) {
		SDL_Log("Missing texture \"%s\"\n", filename.c_str());
		return NULL;
	}

	int name_length = 0;
	fread(&name_length, sizeof(int), 1, file_ptr);

	char name[0xFF] = {};
	if(name_length > 0) {
		memset(name, 0x00, (name_length+1));
		fread(name, sizeof(char), name_length, file_ptr);
	}

	N3TexHeader header_org;
	fread(&header_org, sizeof(N3TexHeader), 1, file_ptr);

	SDL_Log("------------------------------------------------------------");
	SDL_Log("TexName: %s\n", name);
	SDL_Log("H.szID    -> %c%c%c%hu\n",
		header_org.szID[0],
		header_org.szID[1],
		header_org.szID[2],
		(unsigned char) header_org.szID[3]
	);
	SDL_Log("H.nWidth  -> %d\n", header_org.nWidth);
	SDL_Log("H.nHeight -> %d\n", header_org.nHeight);
	SDL_Log("H.Format  -> %d\n", header_org.Format);
	SDL_Log("H.bMipMap -> %d\n", header_org.bMipMap);

	texture_data.header = header_org;

	int pixel_size = 0;
	int comp_tex_size = 0;
	switch(header_org.Format) {
		case D3DFMT_DXT1: {
			comp_tex_size = (header_org.nWidth*header_org.nHeight/2);
		} break;
		case D3DFMT_DXT3:
		case D3DFMT_DXT5: {
			comp_tex_size = (header_org.nWidth*header_org.nHeight);
		} break;
		case D3DFMT_A1R5G5B5:
		case D3DFMT_A4R4G4B4: {
			pixel_size = 2;
		} break;
		case D3DFMT_R8G8B8: {
			pixel_size = 3;
		} break;
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8: {
			pixel_size = 4;
		} break;
		default: {
			SDL_Log("Unknown texture format %d\n", header_org.Format);
			return NULL;
		} break;
	}

	if(pixel_size == 0) {
		texture_data.is_compressed = true;
		texture_data.size = comp_tex_size;
		texture_data.pixels = new unsigned char[comp_tex_size];
		fread(texture_data.pixels, sizeof(unsigned char), comp_tex_size, file_ptr);
	} else {
		texture_data.is_compressed = false;
		texture_data.size = (header_org.nWidth*header_org.nHeight*pixel_size);
		texture_data.pixels = new unsigned char[texture_data.size];
		fread(texture_data.pixels, sizeof(unsigned char), texture_data.size, file_ptr);
	}

	fclose(file_ptr);

	std::pair<std::map<std::string, TextureData>::iterator, bool> ret;
	ret = _texture_data_map.insert(std::make_pair(filename, texture_data));
	if(ret.second == false) {
		SDL_Log("Duplicate!!\n");
		exit(-1);
	}

	return &ret.first->second;
}

//-----------------------------------------------------------------------------
SkinData* ResourceManager::LoadSkinData(const std::string filename) {
	std::map<std::string, SkinData>::iterator iter;

	iter = _skin_data_map.find(filename);
	if(iter != _skin_data_map.end()) {
		return &iter->second;
	}

	SkinData skin_data = {};
	FILE* file_ptr = fopen(filename.c_str(), "rb");
	if(file_ptr == NULL) {
		SDL_Log("Missing skin \"%s\"\n", filename.c_str());
		return NULL;
	}

	int name_length = 0;
	fread(&name_length, sizeof(int), 1, file_ptr);
	char name[0xFF] = {};
	if(name_length > 0) {
		memset(name, 0x00, (name_length+1));
		fread(name, sizeof(char), name_length, file_ptr);
	}
	
	fread(&name_length, sizeof(int), 1, file_ptr);
	if(name_length > 0) {
		memset(name, 0x00, (name_length+1));
		fread(name, sizeof(char), name_length, file_ptr);
	}

	int nFC = 0, nVC = 0, nUVC = 0;
	fread(&nFC, sizeof(int), 1, file_ptr);
	fread(&nVC, sizeof(int), 1, file_ptr);
	fread(&nUVC, sizeof(int), 1, file_ptr);

	if(nFC>0 && nVC>0) {
		if(nFC<=0 || nVC<=0) return false;

		skin_data.face_count = nFC;
		skin_data.vert_count = nVC;

		skin_data.verts = new N3VertexXyzNormal[nVC];
		memset(skin_data.verts, 0, sizeof(N3VertexXyzNormal)*nVC);
		skin_data.indices = new unsigned short[nFC*3];
		memset(skin_data.indices, 0, 2*nFC*3);
		skin_data.skinned_verts = new N3VertexSkinned[nVC];
		memset(skin_data.skinned_verts, 0, sizeof(N3VertexSkinned)*nVC);

		if(nUVC > 0) {
			skin_data.uv_count = nUVC;
			skin_data.uvs = new float[nUVC*2];
			memset(skin_data.uvs, 0, 8*nUVC);
			skin_data.uv_indices = new unsigned short[nFC*3];
			memset(skin_data.uv_indices, 0, 2*nFC*3);
		}

		fread(skin_data.verts, sizeof(N3VertexXyzNormal), nVC, file_ptr);
		fread(skin_data.indices, 2*3, nFC, file_ptr);
	}

	if(skin_data.uv_count > 0) {
		fread(skin_data.uvs, 8, nUVC, file_ptr);
		fread(skin_data.uv_indices, 2*3, nFC, file_ptr);
	}

	for(int i=0; i<nVC; ++i) {
		fread(&skin_data.skinned_verts[i], sizeof(N3VertexSkinned), 1, file_ptr);
		skin_data.skinned_verts[i].pnJoints = NULL;
		skin_data.skinned_verts[i].pfWeights = NULL;

		int nAffect = skin_data.skinned_verts[i].nAffect;
		if(nAffect > 1) {
			skin_data.skinned_verts[i].pnJoints = new int[nAffect];
			skin_data.skinned_verts[i].pfWeights = new float[nAffect];

			fread(skin_data.skinned_verts[i].pnJoints, sizeof(int), nAffect, file_ptr);
			fread(skin_data.skinned_verts[i].pfWeights, sizeof(float), nAffect, file_ptr);

		} else if(nAffect == 1) {
			skin_data.skinned_verts[i].pnJoints = new int[1];
			fread(skin_data.skinned_verts[i].pnJoints, sizeof(int), 1, file_ptr);
		}
	}

	fclose(file_ptr);

	std::pair<std::map<std::string, SkinData>::iterator, bool> ret;
	ret = _skin_data_map.insert(std::make_pair(filename, skin_data));
	if(ret.second == false) {
		SDL_Log("Duplicate!!\n");
		exit(-1);
	}

	return &ret.first->second;
}

//-----------------------------------------------------------------------------
AnimData* ResourceManager::LoadAnimData(const std::string filename) {
	std::map<std::string, AnimData>::iterator iter;

	iter = _anim_data_map.find(filename);
	if(iter != _anim_data_map.end()) {
		return &iter->second;
	}

	AnimData anim_data = {};
	FILE* file_ptr = fopen(filename.c_str(), "rb");
	if(file_ptr == NULL) {
		SDL_Log("Missing anim \"%s\"\n", filename.c_str());
		return NULL;
	}

	int count = 0;
	fread(&count, sizeof(int), 1, file_ptr);

	for(int i=0; i<count; ++i) {
		int nl = 0;
		fread(&nl, sizeof(int), 1, file_ptr);

		float fFrmStart = 0.0f;
		fread(&fFrmStart, sizeof(float), 1, file_ptr);
		float fFrmEnd = 0.0f;
		fread(&fFrmEnd, sizeof(float), 1, file_ptr);
		float fFrmPerSec = 0.0f;
		fread(&fFrmPerSec, sizeof(float), 1, file_ptr);

		float fFrmPlugTraceStart = 0.0f;
		fread(&fFrmPlugTraceStart, sizeof(float), 1, file_ptr);
		float fFrmPlugTraceEnd = 0.0f;
		fread(&fFrmPlugTraceEnd, sizeof(float), 1, file_ptr);

		float fFrmSound0 = 0.0f;
		fread(&fFrmSound0, sizeof(float), 1, file_ptr);
		float fFrmSound1 = 0.0f;
		fread(&fFrmSound1, sizeof(float), 1, file_ptr);

		float fTimeBlend = 0.0f;
		fread(&fTimeBlend, sizeof(float), 1, file_ptr);
		int iBlendFlags = 0;
		fread(&iBlendFlags, sizeof(int), 1, file_ptr);
		float fFrmStrike0 = 0.0f;
		fread(&fFrmStrike0, sizeof(float), 1, file_ptr);
		float fFrmStrike1 = 0.0f;
		fread(&fFrmStrike1, sizeof(float), 1, file_ptr);

		int name_length = 0;
		fread(&name_length, sizeof(int), 1, file_ptr);
		char name[0xFF] = {};
		if(name_length > 0) {
			memset(name, 0x00, (name_length+1));
			fread(name, sizeof(char), name_length, file_ptr);
		}
	}

	fclose(file_ptr);

	std::pair<std::map<std::string, AnimData>::iterator, bool> ret;
	ret = _anim_data_map.insert(std::make_pair(filename, anim_data));
	if(ret.second == false) {
		SDL_Log("Duplicate!!\n");
		exit(-1);
	}

	return &ret.first->second;
}

//-----------------------------------------------------------------------------
JointData* ResourceManager::LoadJointData(const std::string filename) {
	std::map<std::string, JointData>::iterator iter;

	iter = _joint_data_map.find(filename);
	if(iter != _joint_data_map.end()) {
		return &iter->second;
	}

	JointData joint_data = {};
	FILE* file_ptr = fopen(filename.c_str(), "rb");
	if(file_ptr == NULL) {
		SDL_Log("Missing joint \"%s\"\n", filename.c_str());
		return NULL;
	}

	joint_data.load(file_ptr);

	fclose(file_ptr);

	std::pair<std::map<std::string, JointData>::iterator, bool> ret;
	ret = _joint_data_map.insert(std::make_pair(filename, joint_data));
	if(ret.second == false) {
		SDL_Log("Duplicate!!\n");
		exit(-1);
	}

	return &ret.first->second;
}
