#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

#include <filesystem>
#include <vector>

namespace Dooky {
	struct FileThumbnailImage {
		bool success;
		int width;
		int height;
		std::vector<unsigned char> bitmap;
	};

	FileThumbnailImage GetImageFileThumbnail(const std::filesystem::path& path, int targetSize);
}


#endif