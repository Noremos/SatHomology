module;

// Side
#include "PortFileDialog.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"



export module SideBind;

export stbi_uc* stbi_load(char const* filename, int* x, int* y, int* comp, int req_comp);
export void stbi_image_free(void* retval_from_stbi_load);
export int stbi_write_png(char const* filename, int x, int y, int comp, const void* data, int stride_bytes);
export stbi_uc* stbi_load_from_memory(stbi_uc const* buffer, int len, int* x, int* y, int* comp, int req_comp);
export unsigned char* stbi_write_png_to_mem(const unsigned char* pixels, int stride_bytes, int x, int y, int n, int* out_len);
