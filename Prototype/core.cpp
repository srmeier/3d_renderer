/*
*/

#include "core.h"

//-----------------------------------------------------------------------------
char* load_file_into_buffer(const char* file_name) {
	FILE* file_ptr = fopen(file_name, "rb");
	if(!file_ptr) {
		SDL_Log("Unable to open file \"%s\"\n", file_name);
		return NULL;
	}

	fseek(file_ptr, 0L, SEEK_END);
	int file_size = ftell(file_ptr);
	fseek(file_ptr, 0L, SEEK_SET);

	char* buffer = (char*)calloc((file_size+1), sizeof(char));
	fread(buffer, sizeof(char), file_size, file_ptr);

	fclose(file_ptr);

	return buffer;
}
