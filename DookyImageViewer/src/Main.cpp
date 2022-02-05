#include "Application.h"

#include <Windows.h>
#include <direct.h>
#include <filesystem>
#include <fstream>
#include <Magick++.h>

int wmain(int argc, wchar_t** argv) {
    std::string parentPath = std::filesystem::path(argv[0]).parent_path().string();
    std::wstring wideParentPath = std::filesystem::path(argv[0]).parent_path().wstring();

    // Set ImageMagick API Environment Variables

    _putenv_s("HOME",                     (parentPath + "\\ImageMagickResources").c_str()                   );
    _putenv_s("LD_LIBRARY_PATH",          (parentPath + "\\ImageMagickResources").c_str()                   );
    _putenv_s("MAGICK_HOME",              (parentPath + "\\ImageMagickResources").c_str()                   );
    _putenv_s("MAGICK_CONFIGURE_PATH",    (parentPath + "\\ImageMagickResources\\configure").c_str()        );
    _putenv_s("MAGICK_CODER_MODULE_PATH", (parentPath + "\\ImageMagickResources\\modules\\coders").c_str()  );
    _putenv_s("MAGICK_CODER_FILTER_PATH", (parentPath + "\\ImageMagickResources\\modules\\filters").c_str() );

    // Change working directory
    // Reason: to make it so that ImageMagick still works even if you open the program
    //         with a file dragged and dropped onto it as before it would not work

    char convertedPath[1024];
    WideCharToMultiByte(65001, 0, wideParentPath.c_str(), -1, convertedPath, 1024, NULL, NULL);

    if (_chdir(convertedPath))
        printf("Successfully changed working directory.\n");

    char current[1024];

    if (_getcwd(current, sizeof(current))) {
        printf("Current: %s\n", current);
        printf("Working: %s\n", convertedPath);
    } else {
        printf("Failed to retrieve current working directory.\n");
    }

    // Read image viewer config
    if (!std::filesystem::exists("imageviewerconfig.ini")) {
        std::ofstream newConfig("imageviewerconfig.ini");
        newConfig << "hideconsole" << std::endl;
        newConfig.close();
    }

    std::fstream configStream("imageviewerconfig.ini");
    std::string line;

    if (configStream.is_open()) {
        while (std::getline(configStream, line)) {
            if (line == "hideconsole") {
                FreeConsole();
            }
        }
    } else {
        printf("Failed to open 'imageviewerconfig.ini'\n");
    }

    // Initialize Magick++
    
    //Magick::InitializeMagick(*argv);

    // Begin Application

    Dooky::Application app;
    app.Begin(argc, argv);
}