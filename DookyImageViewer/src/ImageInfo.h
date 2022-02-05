#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <string>
#include <filesystem>
#include "vendor/TinyEXIF/TinyEXIF.h"

namespace Dooky {
	size_t GetFileLastModifiedTime(const std::filesystem::path& path);
	std::string GetFileLastModifiedTimestampString(const std::filesystem::path& path);

	TinyEXIF::EXIFInfo GetImageExifData(const std::filesystem::path& path);
	std::string GetImageExifInformationString(const TinyEXIF::EXIFInfo& imageEXIF);
}

#endif