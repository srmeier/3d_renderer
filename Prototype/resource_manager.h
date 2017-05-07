#ifndef __RESOURCE_MANAGER_H_
#define __RESOURCE_MANAGER_H_

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

//-----------------------------------------------------------------------------
class ResourceManager {
private:
	static std::map<std::string, MeshData> _mesh_data_map;
	static std::map<std::string, TextureData> _texture_data_map;

public:
	static MeshData* LoadMeshData(const std::string filename);
	static TextureData* LoadTextureData(const std::string filename);
};

#endif
