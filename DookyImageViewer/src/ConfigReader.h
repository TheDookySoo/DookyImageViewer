#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <string>


struct Config {
	bool hideConsole;
	bool useMipmaps;
} typedef Config;

Config ReadConfigFile(const std::string& path);

#endif