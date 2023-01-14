#include "GuiCommon.h"

void ResizeImage(int& width, int& height, int max_width, int max_height)
{
	// Calculate the aspect ratio of the image
	float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

	// Calculate the maximum width and height that maintain the aspect ratio
	int max_aspect_width = std::round(max_height * aspect_ratio);
	int max_aspect_height = std::round(max_width / aspect_ratio);

	// Use the maximum width or height that maintains the aspect ratio, whichever is smaller
	width = std::min(max_width, max_aspect_width);
	height = std::min(max_height, max_aspect_height);
}
