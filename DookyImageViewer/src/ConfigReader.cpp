#include "ConfigReader.h"

#include <fstream>
#include <filesystem>


Config ReadConfigFile(const std::string& path) {
    Config config;

    // Defaults
    config.hideConsole = false;
    config.useMipmaps = false;

    // Create new default config if the file doesn't already exist
    if (!std::filesystem::exists(path)) {
        std::ofstream newConfig(path);
        newConfig << "hideconsole" << std::endl;
        newConfig << "usemipmaps" << std::endl;
        newConfig.close();
    }

    // Read the config file
    std::fstream configStream("imageviewerconfig.ini");
    std::string line;

    if (configStream.is_open()) {
        while (std::getline(configStream, line)) {
            if (line == "hideconsole") {
                config.hideConsole = true;
            } else if (line == "usemipmaps") {
                config.useMipmaps = true;
            }
        }
    } else {
        printf("Failed to open 'imageviewerconfig.ini'\n");
    }

    return config;
}