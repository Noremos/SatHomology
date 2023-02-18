module;

// Side
#include "../side/PortFileDialog.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../side/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../side/stb_image_write.h"



export module SideBind;

export stbi_uc* stbi_load(char const* filename, int* x, int* y, int* comp, int req_comp);
export void stbi_image_free(void* retval_from_stbi_load);
export int stbi_write_png(char const* filename, int x, int y, int comp, const void* data, int stride_bytes);
