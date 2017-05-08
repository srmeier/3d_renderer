#ifndef __RESOURCE_MANAGER_H_
#define __RESOURCE_MANAGER_H_

// TODO: free these!!
//-----------------------------------------------------------------------------
typedef struct {
	int num_verts;
	int num_indices;
	N3VertexT1* verts;
	unsigned short* indices;
} MeshData;

typedef struct {
	int size;
	N3TexHeader header;
	bool is_compressed;
	unsigned char* pixels;
} TextureData;

typedef struct {
	int face_count;
	int vert_count;
	int uv_count;
	N3VertexXyzNormal* verts;
	unsigned short* indices;
	float* uvs;
	unsigned short* uv_indices;
	N3VertexSkinned* skinned_verts;
} SkinData;

typedef struct {
} AnimData;

// TODO: this should probably be a class...
typedef struct _JointData {
	std::vector<struct _JointData> children;

	int pos_key_count;
	glm::vec3* pos_keys;
	float pos_sampling_rate;
	int rot_key_count;
	glm::quat* rot_keys;
	float rot_sampling_rate;

	int scale_key_count;
	int orient_key_count;

	bool getPos(float frame, glm::vec3& vec) {
		if(pos_key_count <= 0) return false;

		float fD = 30.0f/pos_sampling_rate;
		int index = (int)(frame*(pos_sampling_rate/30.0f));
		if(index<0 || index>pos_key_count) return false;

		float fDelta = 0.0f;
		if(index == pos_key_count) {
			index = pos_key_count-1;
			fDelta = 0.0f;
		} else fDelta = (frame-index*fD)/fD;

		if(fDelta != 0.0f) {
			vec = (pos_keys[index]*(1.0f-fDelta)) + (pos_keys[index+1]*fDelta);
		} else {
			vec = pos_keys[index];
		}

		return true;
	}

	bool getRot(float frame, glm::quat& quat) {
		if(rot_key_count <= 0) return false;

		float fD = 30.0f/rot_sampling_rate;
		int index = (int)(frame*(rot_sampling_rate/30.0f));
		if(index<0 || index>rot_key_count) return false;

		float fDelta = 0.0f;
		if(index == rot_key_count) {
			index = rot_key_count-1;
			fDelta = 0.0f;
		} else fDelta = (frame-index*fD)/fD;

		if(fDelta != 0.0f) {
			quat = glm::slerp(rot_keys[index], rot_keys[index+1], fDelta);
		} else {
			quat = rot_keys[index];
		}

		return true;
	}

	void load(FILE* file_ptr) {
		int name_length = 0;
		fread(&name_length, sizeof(int), 1, file_ptr);
		char name[0xFF] = {};
		if(name_length > 0) {
			memset(name, 0x00, (name_length+1));
			fread(name, sizeof(char), name_length, file_ptr);
		}

		glm::vec3 m_vPos;
		fread(&m_vPos, sizeof(glm::vec3), 1, file_ptr);
		glm::quat m_qRot;
		fread(&m_qRot, sizeof(glm::quat), 1, file_ptr);
		glm::vec3 m_vScale;
		fread(&m_vScale, sizeof(glm::vec3), 1, file_ptr);

		// m_KeyPos.Load(hFile);
		pos_key_count = 0;
		fread(&pos_key_count, sizeof(int), 1, file_ptr);
		if(pos_key_count > 0) {
			ANIMATION_KEY_TYPE m_eType;
			fread(&m_eType, sizeof(ANIMATION_KEY_TYPE), 1, file_ptr);
			pos_sampling_rate = 0.0f;
			fread(&pos_sampling_rate, sizeof(float), 1, file_ptr);

			if(m_eType == KEY_VECTOR3) {
				pos_keys = new glm::vec3[(pos_key_count+1)];
				memset(pos_keys, 0x00, sizeof(glm::vec3)*(pos_key_count+1));

				fread(pos_keys, sizeof(glm::vec3), pos_key_count, file_ptr);
				pos_keys[pos_key_count] = pos_keys[pos_key_count-1];
			} else {
				SDL_Log("Wrong animation type\n");
				exit(-1);
			}
		}

		// m_KeyRot.Load(hFile);
		rot_key_count = 0;
		fread(&rot_key_count, sizeof(int), 1, file_ptr);
		if(rot_key_count > 0) {
			ANIMATION_KEY_TYPE m_eType;
			fread(&m_eType, sizeof(ANIMATION_KEY_TYPE), 1, file_ptr);
			rot_sampling_rate = 0.0f;
			fread(&rot_sampling_rate, sizeof(float), 1, file_ptr);

			if(m_eType == KEY_QUATERNION) {
				rot_keys = new glm::quat[(rot_key_count+1)];
				memset(rot_keys, 0x00, sizeof(glm::quat)*(rot_key_count+1));

				fread(rot_keys, sizeof(glm::quat), rot_key_count, file_ptr);
				rot_keys[rot_key_count] = rot_keys[rot_key_count-1];
			} else {
				SDL_Log("Wrong animation type\n");
				exit(-1);
			}
		}

		// m_KeyScale.Load(hFile);
		scale_key_count = 0;
		fread(&scale_key_count, sizeof(int), 1, file_ptr);
		if(scale_key_count > 0) {
			ANIMATION_KEY_TYPE m_eType;
			fread(&m_eType, sizeof(ANIMATION_KEY_TYPE), 1, file_ptr);
			float m_fSamplingRate = 0.0f;
			fread(&m_fSamplingRate, sizeof(float), 1, file_ptr);

			if(m_eType == KEY_VECTOR3) {
				glm::vec3* keys = new glm::vec3[(scale_key_count+1)];
				memset(keys, 0x00, sizeof(glm::vec3)*(scale_key_count+1));

				fread(keys, sizeof(glm::vec3), scale_key_count, file_ptr);
				keys[scale_key_count] = keys[scale_key_count-1];

			} else if(m_eType == KEY_QUATERNION) {
				glm::quat* keys = new glm::quat[(scale_key_count+1)];
				memset(keys, 0x00, sizeof(glm::quat)*(scale_key_count+1));

				fread(keys, sizeof(glm::quat), scale_key_count, file_ptr);
				keys[scale_key_count] = keys[scale_key_count-1];
			}
		}

		//m_KeyOrient.Load(hFile);
		orient_key_count = 0;
		fread(&orient_key_count, sizeof(int), 1, file_ptr);
		if(orient_key_count > 0) {
			ANIMATION_KEY_TYPE m_eType;
			fread(&m_eType, sizeof(ANIMATION_KEY_TYPE), 1, file_ptr);
			float m_fSamplingRate = 0.0f;
			fread(&m_fSamplingRate, sizeof(float), 1, file_ptr);

			if(m_eType == KEY_VECTOR3) {
				glm::vec3* keys = new glm::vec3[(orient_key_count+1)];
				memset(keys, 0x00, sizeof(glm::vec3)*(orient_key_count+1));

				fread(keys, sizeof(glm::vec3), orient_key_count, file_ptr);
				keys[orient_key_count] = keys[orient_key_count-1];

			} else if(m_eType == KEY_QUATERNION) {
				glm::quat* keys = new glm::quat[(orient_key_count+1)];
				memset(keys, 0x00, sizeof(glm::quat)*(orient_key_count+1));

				fread(keys, sizeof(glm::quat), orient_key_count, file_ptr);
				keys[orient_key_count] = keys[orient_key_count-1];
			}
		}

		int nCC = 0;
		fread(&nCC, sizeof(int), 1, file_ptr);
		for(int i=0; i<nCC; ++i) {
			JointData data = {};
			data.load(file_ptr);

			children.push_back(data);
		}
	}
} JointData;

//-----------------------------------------------------------------------------
class ResourceManager {
private:
	static std::map<std::string, MeshData> _mesh_data_map;
	static std::map<std::string, TextureData> _texture_data_map;
	static std::map<std::string, SkinData> _skin_data_map;
	static std::map<std::string, AnimData> _anim_data_map;
	static std::map<std::string, JointData> _joint_data_map;

public:
	static MeshData* LoadMeshData(const std::string filename);
	static TextureData* LoadTextureData(const std::string filename);
	static SkinData* LoadSkinData(const std::string filename);
	static AnimData* LoadAnimData(const std::string filename);
	static JointData* LoadJointData(const std::string filename);
};

#endif
