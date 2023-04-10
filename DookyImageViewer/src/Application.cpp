#include "Application.h"
#include "Window.h"
#include "Image.h"
#include "Text.h"
#include "ImageUtils.h"
#include "ImageInfo.h"
#include "ThumbnailPreview.h"
#include "GUI.h"
#include "StringUtils.h"

#include "vendor/portable-file-dialogs/portable-file-dialogs.h"

#include <unordered_set>
#include <Magick++.h>

#include "vendor/stb_image/stb_image.h"


std::unordered_set<std::string> RECOGNISED_IMAGE_EXTENSIONS = {
    ".jpg", ".jpeg", ".jfif", ".pjpeg", ".pjp", // JPEG
    ".png", ".apng", // PNG
    ".tga", ".icb", ".vda", ".vst", // TGA
    ".bmp", // BMP
    ".psd", // PSD
    ".gif", // GIF
    ".hdr", // HDR
    ".pic", // Softimage PIC
    ".pbm", ".pgm", ".ppm", ".pnm", // Portable Any Map
    ".exr", // EXR
    ".cr2", ".crw", // Canon RAW
    ".dcr", // Kodak RAW
    ".heic", // Apple High Efficiency Image Format
    ".webp", // WEBP
    ".mrw", // Minolta RAW
    ".arw", // Sony RAW
    ".nef", // Nikon RAW
    ".orf", // Olympus RAW
    ".raf", // Fuji RAW
    ".rgf", // LEGO Mindstorms EV3 Robot Graphics File
    ".rla", // Wavefront image file
    ".svg", // Scalable Vector Graphics
    ".tif", ".tiff", // TIFF
    ".miff", // Magick Image File Format
    ".ttf", // TrueType Font
    ".otf", // OpenType Font
    ".xcf", // GIMP Image
    ".x3f", // Sigma RAW
    ".wpg", // WordPerfect Graphic
    ".wdp", // Windows Media Photo File
    ".viff", // Visualization Image File Format
    ".vicar", // NASA Jet Propulsion Laboratory's Image Format
    ".sfw", // Seattle FilmWorks Image
    ".sct", // Scitex Continuous Tone File
    ".rle", // Run Length Encoded Bitmap
    ".bpg", // Better Portable Graphics
    ".cur", // Microsoft Cursor Icon
    ".dcx", // Zsoft Multi-Page Paintbrush Image File
    ".ico" // Windows Icon Image Format
};

// CONSTANTS

float ZOOM_INCREMENT = 1.25f;
int INFORMATION_BAR_HEIGHT = 19;
int THUMBNAIL_PREVIEW_WITH_MENU_BAR_HEIGHT = 91;
int MENU_BAR_HEIGHT = 19;
glm::ivec2 ZOOM_TEXT_OFFSET = { 4, -5 };

// VARIABLES

glm::ivec4 mainImagePermissibleBoundary = { 0, 0, 100, 100 };
glm::ivec2 mainImagePosition = { 0, 0 };
float mainImageZoom = 1.0f;
int mainImageRotation = 0;
bool mainImageEngaged = false;
bool mainImageFailedToLoad = false;
bool mainImageDragging = false;
size_t mainImageFileSize = 0;

bool hotkeyShouldOpenFile = false;
bool hotkeyShouldOpenDirectory = false;
bool hotkeyShouldOpenSubdirectories = false;

glm::ivec2 lastMouseDownPosition = { 0, 0 };
float lastMouseDownTime = 0.0f;

int browsingListIndex = 0;
int browsingListSortMode = 0;
bool browsed = false;
std::vector<std::filesystem::path> browsingList;
std::filesystem::path mainImageCurrentFilePath;
std::stringstream fileSizeStr;

Dooky::Text* errorMessageText;

size_t GetFileSize(const std::filesystem::path& path) {
    std::ifstream input(path, std::ifstream::ate | std::ifstream::binary);
    return input.tellg();
}

// APPLICATION

namespace Dooky {
    ////////////////////////////////////////
    ///// APPLICATION FUNCTIONS
    ////////////////////////////////////////

    void FitImageOnScreen(Window& window, Image& mainImage) {
        glm::ivec2 windowSize = window.GetSize();
        glm::ivec2 imageSize = mainImage.GetSize();

        // Fit rotated image correctly

        if (mainImageRotation == -90 || mainImageRotation == 90 || mainImageRotation == -270 || mainImageRotation == 270) {
            imageSize = { imageSize.y, imageSize.x };
        }

        glm::ivec2 boundarySize = { mainImagePermissibleBoundary[2] - mainImagePermissibleBoundary[0], mainImagePermissibleBoundary[3] - mainImagePermissibleBoundary[1] };

        if (imageSize.x > boundarySize.x || imageSize.y > boundarySize.y) {
            float screenAspect = (float)boundarySize.x / (float)boundarySize.y;
            float imageAspect = (float)imageSize.x / (float)imageSize.y;

            float scaleFactor = 1.0f;

            if (screenAspect > imageAspect) {
                scaleFactor = (float)boundarySize.y / (float)imageSize.y;
            } else {
                scaleFactor = (float)boundarySize.x / (float)imageSize.x;
            }

            mainImageZoom = scaleFactor;
        } else {
            mainImageZoom = 1.0f;
        }

        mainImageEngaged = true;
        mainImagePosition = { (float)boundarySize.x / 2.0f + mainImagePermissibleBoundary[0], (float)boundarySize.y / 2.0f + mainImagePermissibleBoundary[1] };
    }

    void HandleImageOpenFail(Image& mainImage, GUI& gui) {
        mainImageFailedToLoad = true;
        mainImageRotation = 0;

        gui.imageInformationText = "";
        gui.imageInformationExifText = "";
        gui.menuBarExtraTextColor = MENU_BAR_EXTRA_TEXT_COLOR_ERROR;

        errorMessageText->SetString("Failed to open this image.");
        fileSizeStr = std::stringstream();

        std::string fileNameStr = "CAN'T DISPLAY FILE NAME!";
        std::string browsingIndex = std::to_string(browsingListIndex + 1) + "/" + std::to_string(browsingList.size());

        if (browsingList.empty())
            browsingIndex = "EMPTY BROWSING LIST";

        try {
            fileNameStr = mainImageCurrentFilePath.filename().string();
        } catch (std::system_error& exception) {}

        gui.menuBarText = "| " + browsingIndex + " | " + fileNameStr;
    }

    void HandlePostImageLoad(Window& window, Image& mainImage, GUI& gui, const std::filesystem::path& loadedPath) {
        mainImageFailedToLoad = false;
        mainImageRotation = 0;

        // Reset
        gui.imageInformationText = "";
        gui.imageInformationExifText = "";
        gui.menuBarExtraTextColor = MENU_BAR_EXTRA_TEXT_COLOR_NORMAL;

        // Read EXIF
        TinyEXIF::EXIFInfo exif = GetImageExifData(mainImageCurrentFilePath);

        if (exif.Fields) {
            switch (exif.Orientation) {
            case 1: mainImageRotation = 0; break;
            case 2: mainImageRotation = 0; break;
            case 3: mainImageRotation = 180; break;
            case 4: mainImageRotation = -180; break;
            case 5: mainImageRotation = 90; break;
            case 6: mainImageRotation = -90; break;
            case 7: mainImageRotation = 270; break;
            case 8: mainImageRotation = -270; break;
            }

            std::string exifInfo = GetImageExifInformationString(exif);

            gui.imageInformationExifText = exifInfo;
        }

        mainImageFileSize = GetFileSize(loadedPath);

        // Information text
        std::string informationText;

        std::string browsingIndex = std::to_string(browsingListIndex + 1) + "/" + std::to_string(browsingList.size());
        std::string resolution = std::to_string(mainImage.GetSize().x) + "x" + std::to_string(mainImage.GetSize().y);
        std::string fileNameStr = "CAN'T DISPLAY FILE NAME!";
        std::string modifiedDateStr = GetFileLastModifiedTimestampString(mainImageCurrentFilePath);

        fileSizeStr = std::stringstream();
        fileSizeStr << "File size: ";

        try {
            fileSizeStr.imbue(std::locale(""));
            fileSizeStr << std::fixed << GetFileSize(loadedPath) << " bytes";
        } catch (std::system_error& exception) {}

        try {
            fileNameStr = mainImageCurrentFilePath.filename().string();
        } catch (std::system_error& exception) {}

        informationText += "Dimensions: " + std::to_string(mainImage.GetSize().x) + "x" + std::to_string(mainImage.GetSize().y) + "\n";
        informationText += "Last Modified Time: " + modifiedDateStr;

        gui.imageInformationText = informationText;
        gui.menuBarText = "| " + browsingIndex + " | " + fileNameStr + " | " + resolution + " | " + modifiedDateStr;

        window.SetTitle(mainImageCurrentFilePath.filename().wstring());

        // Fit image on screen
        FitImageOnScreen(window, mainImage);
    }
    
    // Sort
    void SortBrowsingList(int sortType) {
        if (sortType == 0) { // Alphabetical

        } else if (sortType == 1) { // Modified date (descending)
            std::sort(browsingList.begin(), browsingList.end(), [](const std::filesystem::path& left, const std::filesystem::path& right) {
                size_t leftTime = GetFileLastModifiedTime(left);
                size_t rightTime = GetFileLastModifiedTime(right);

                return leftTime > rightTime;
            });
        }
    }

    // Only changes the image by itself
    bool ChangeImage(Window& window, Image& mainImage, GUI& gui, const std::filesystem::path& imagePath) {
        if (mainImage.LoadImageFile(imagePath)) {
            HandlePostImageLoad(window, mainImage, gui, imagePath);
            return true;
        } else {
            //std::cout << "ERROR: Failed to change image: " << imagePath.string() << std::endl;
            HandleImageOpenFail(mainImage, gui);
            return false;
        }
    }

    // Opens either an image or directory and also searches the directory/subdirectories
    bool OpenNewPath(Window& window, Image& mainImage, GUI& gui, const std::filesystem::path& openPath, bool supportedExtensionsOnly, bool openSubdirectories) {
        if (!std::filesystem::exists(openPath)) {
            std::cout << "ERROR: Couldn't open new path as it doesn't exist." << std::endl;
            return false;
        }

        std::vector<std::filesystem::path> newBrowsingList;

        auto AttemptAddPathToBrowsingList = [&](const std::filesystem::path& searchPath) {
            try {
                if (std::filesystem::is_regular_file(searchPath)) {
                    std::string loweredExtension = searchPath.extension().string();
                    LowerString(loweredExtension);

                    if (supportedExtensionsOnly == false || RECOGNISED_IMAGE_EXTENSIONS.contains(loweredExtension))
                        newBrowsingList.push_back(searchPath);
                }
            } catch (std::exception& exception) {
                std::cout << "EXCEPTION: " << " when opening " << searchPath.filename() << ": " << exception.what() << std::endl;
            }
        };

        if (std::filesystem::is_directory(openPath)) {
            if (openSubdirectories) {
                for (auto path : std::filesystem::recursive_directory_iterator(openPath)) {
                    AttemptAddPathToBrowsingList(path.path());
                }
            } else {
                for (auto path : std::filesystem::directory_iterator(openPath)) {
                    AttemptAddPathToBrowsingList(path.path());
                }
            }

            if (!newBrowsingList.empty()) {
                browsingList = newBrowsingList;
                SortBrowsingList(browsingListSortMode);

                // Try opening the first file in the directory
                std::filesystem::path front = browsingList.front();
                mainImageCurrentFilePath = front;
                browsingListIndex = 0;


                if (mainImage.LoadImageFile(front)) {
                    HandlePostImageLoad(window, mainImage, gui, front);
                } else {
                    HandleImageOpenFail(mainImage, gui);
                    std::cout << "ERROR: Failed to load first image in newBrowsingList: " << openPath.string() << std::endl;
                }

                return true;
            } else {
                std::cout << "ERROR: newBrowsingList is empty." << std::endl;
                HandleImageOpenFail(mainImage, gui);
                errorMessageText->SetString("This directory has no images.");

                return false;
            }
        } else if (std::filesystem::is_regular_file(openPath)) {
            // Search for other images in the same directory
            for (auto path : std::filesystem::directory_iterator(openPath.parent_path()))
                AttemptAddPathToBrowsingList(path.path());

            browsingList = newBrowsingList; // Has to be at least one path inside
            SortBrowsingList(browsingListSortMode);

            // Find index
            int newBrowsingListIndex = 0;

            for (int i = 0; i < browsingList.size(); i++) {
                auto path = browsingList[i];

                if (std::filesystem::equivalent(path, openPath)) {
                    newBrowsingListIndex = i;
                    break;
                }
            }

            std::filesystem::path openPath2 = browsingList[newBrowsingListIndex];
            mainImageCurrentFilePath = openPath2;

            // Load the image
            if (mainImage.LoadImageFile(openPath2)) {
                browsingListIndex = newBrowsingListIndex;

                // Reset image stuff
                HandlePostImageLoad(window, mainImage, gui, openPath2);
                FitImageOnScreen(window, mainImage);

                return true;
            } else {
                HandleImageOpenFail(mainImage, gui);
                std::cout << "ERROR: Failed to load image: " << openPath2.string() << std::endl;

                return false;
            }
        }
    }

    void UpdateMainImage(Window& window, Image& mainImage) {
        // Engaged
        if (mainImageEngaged) {
            FitImageOnScreen(window, mainImage);
        }

        // Keep image within borders
        glm::ivec2 windowSize = window.GetSize();
        glm::ivec2 imageSize = mainImage.GetSize();

        int imageBoundsX = (imageSize.x / 2) * mainImageZoom - (windowSize.x / 10);
        int imageBoundsY = (imageSize.y / 2) * mainImageZoom - (windowSize.y / 10);

        if (mainImagePosition.x < -imageBoundsX) mainImagePosition.x = -imageBoundsX;
        if (mainImagePosition.x > windowSize.x + imageBoundsX) mainImagePosition.x = windowSize.x + imageBoundsX;

        if (mainImagePosition.y < -imageBoundsY) mainImagePosition.y = -imageBoundsY;
        if (mainImagePosition.y > windowSize.y + imageBoundsY) mainImagePosition.y = windowSize.y + imageBoundsY;

        // Update
        mainImage.SetScale(mainImageZoom, mainImageZoom);
        mainImage.SetPosition(mainImagePosition.x, mainImagePosition.y);
        mainImage.SetRotation(mainImageRotation);
    }
    
    void HandleImageInteraction(Window& window, Image& mainImage, ThumbnailPreview& thumbnails, GUI& gui) {
        glm::ivec2 windowSize = window.GetSize();
        glm::ivec2 mouseScrollDelta = window.GetMouseScrollDelta();
        glm::ivec2 mouseMoveDelta = window.GetMouseMoveDelta();
        glm::ivec2 mousePosition = window.GetMousePosition();

        bool mouseWithinPermissibleBoundary = false;

        {
            glm::ivec2 p = mousePosition;
            glm::ivec4 b = mainImagePermissibleBoundary;

            if (p.x >= b[0] && p.x <= b[2] && p.y >= b[1] && p.y <= b[3])
                mouseWithinPermissibleBoundary = true;
        }

        // Drag and drop
        auto droppedPaths = window.GetDroppedPaths();

        if (!droppedPaths.empty()) {
            auto front = droppedPaths.front();
            OpenNewPath(window, mainImage, gui, front, true, false);
            thumbnails.ChangeBrowsingListAndIndex(browsingList, browsingListIndex);
        }

        // Ignore image manipulation if failed to load
        if (mainImageFailedToLoad)
            return;

        // Drag Image
        bool button1Pressed = window.WasMousePressed(GLFW_MOUSE_BUTTON_1);
        bool button2Pressed = window.WasMousePressed(GLFW_MOUSE_BUTTON_2);

        bool button1Down = window.IsMouseButtonDown(GLFW_MOUSE_BUTTON_1);
        bool button2Down = window.IsMouseButtonDown(GLFW_MOUSE_BUTTON_2);

        if ((button1Pressed || button2Pressed) && mouseWithinPermissibleBoundary && !gui.imguiCaptureMouse)
            mainImageDragging = true;

        if (!button1Down && !button2Down)
            mainImageDragging = false;

        if (mainImageDragging) {
            if (mouseMoveDelta.x != 0 || mouseMoveDelta.y != 0) {
                mainImageEngaged = false;
            }

            mainImagePosition -= mouseMoveDelta;
        }

        // Rotate Image
        if (window.WasKeyFired(GLFW_KEY_R) && !gui.imguiCaptureKeyboard) {
            mainImageRotation -= 90;

            if (mainImageRotation <= -360) {
                mainImageRotation += 360;
            }

            if (mainImageEngaged) {
                FitImageOnScreen(window, mainImage);
            }
        }

        // Zoom
        if (mouseScrollDelta.y != 0 && mouseWithinPermissibleBoundary && !gui.imguiCaptureMouse) {
            glm::ivec2 offset = mousePosition - mainImagePosition;

            float factor = ZOOM_INCREMENT;
            float increment = ZOOM_INCREMENT;

            if (window.IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {
                factor = 1.05f;
                increment = 1.05f;
            }

            if (mouseScrollDelta.y < 0) {
                factor = 1 / factor;
            }

            if (mouseScrollDelta.y < 0) {
                mainImagePosition -= glm::ivec2(offset.x * (factor - 1), offset.y * (factor - 1));
                mainImageZoom /= increment;
            } else if (mouseScrollDelta.y > 0) {
                mainImagePosition -= glm::ivec2(offset.x * (factor - 1), offset.y * (factor - 1));
                mainImageZoom *= increment;
            }

            mainImageEngaged = false;

            // Limits
            if (mainImageZoom < 0.00000000000000000000000001f) {
                mainImageZoom = 0.00000000000000000000000001f;
            } else if (mainImageZoom > 100000000000000000000000000.0f) {
                mainImageZoom = 100000000000000000000000000.0f;
            }
        }

        // Double click to zoom to 100%
        if (window.WasMousePressed(GLFW_MOUSE_BUTTON_1) && mouseWithinPermissibleBoundary && !gui.imguiCaptureMouse) {
            if (window.GetTime() - lastMouseDownTime < 0.3f && mousePosition == lastMouseDownPosition) {
                if (mainImageEngaged) {
                    mainImageEngaged = false;

                    mainImageZoom = 1.0f;
                    mainImagePosition = { (float)windowSize.x / 2.0f, (float)windowSize.y / 2.0f };
                } else {
                    FitImageOnScreen(window, mainImage);
                }
            }

            lastMouseDownPosition = mousePosition;
            lastMouseDownTime = window.GetTime();
        }

        // Check if we should use linear interpolation or nearest neighbour
        if (mainImageZoom < 4) { // Once individual pixels start becoming 4 pixels wide then disable linear interpolation
            mainImage.EnableLinearInterpolation(true);
        } else {
            mainImage.EnableLinearInterpolation(false);
        }
    }

    void HandleImageBrowsing(Window& window, Image& mainImage, GUI& gui) {
        if (gui.imguiCaptureKeyboard)
            return;

        int mouseDeltaX = window.GetMouseScrollDelta().x;

        bool leftControlDown = window.IsKeyDown(GLFW_KEY_LEFT_CONTROL);
        bool leftShiftDown = window.IsKeyDown(GLFW_KEY_LEFT_SHIFT);

        if (window.WasKeyFired(GLFW_KEY_RIGHT) || window.WasKeyFired(GLFW_KEY_LEFT) || window.WasKeyFired(GLFW_KEY_COMMA) || window.WasKeyFired(GLFW_KEY_PERIOD) || mouseDeltaX != 0) {
            int prevIndex = browsingListIndex;
            int increment = 1;

            if (leftControlDown) increment = 10;

            if (leftShiftDown) increment = 100;

            if (window.WasKeyFired(GLFW_KEY_RIGHT) || mouseDeltaX < 0) {
                browsingListIndex += increment;

                if (browsingListIndex >= browsingList.size()) {
                    browsingListIndex = 0;
                }
            }

            if (window.WasKeyFired(GLFW_KEY_LEFT) || mouseDeltaX > 0) {
                browsingListIndex -= increment;

                if (browsingListIndex < 0) {
                    browsingListIndex = browsingList.size() - 1;
                }
            }

            if (window.WasKeyFired(GLFW_KEY_COMMA)) browsingListIndex = 0;
            if (window.WasKeyFired(GLFW_KEY_PERIOD)) browsingListIndex = browsingList.size() - 1;

            if (prevIndex == browsingListIndex) return;

            if (browsingList.empty()) return;

            // Change image
            browsed = true;

            mainImageCurrentFilePath = browsingList[browsingListIndex];
            ChangeImage(window, mainImage, gui, mainImageCurrentFilePath);
        }
    }

    void HandleGuiInteraction(Window& window, Image& mainImage, ThumbnailPreview& thumbnails, GUI& gui) {
        // Open single file
        if (gui.wantsToOpenFile || hotkeyShouldOpenFile) {
            auto selection = pfd::open_file(
                "Select a file.",
                ".",
                {
                    "Supported Image Files", "*.jpg *.jpeg *.jfif *.pjpeg *.pjp *.png *.tga *.icb *.vda *.vst *.bmp *.psd *.gif *.hdr *.pic *.pbm *.pgm *.ppm *.pnm *.exr *.cr2 *.crw *.dcr *.heic *.webp *.mrw *.arw *.nef *.orf *.raf *.rgf *.rla *.svg *.tif *.tiff *.miff *.ttf *.xcf *.x3f *.wpg *.wdp *.viff *.vicar *.sfw *.sct *.rle *.bpg *.cur *.dcx *.ico",
                    "All Files", "*"
                }
            );

            auto const& files = selection.result();

            if (!files.empty()) {
                std::string file = files.front();

                int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &file.at(0), (int)file.size(), NULL, 0);
                std::wstring wStr(sizeNeeded, 0);
                MultiByteToWideChar(CP_UTF8, 0, &file.at(0), (int)file.size(), &wStr.at(0), sizeNeeded);

                std::filesystem::path newTestPath = wStr;

                if (std::filesystem::exists(newTestPath)) {
                    OpenNewPath(window, mainImage, gui, newTestPath, gui.ignoreUnknownFileExtensions, false);
                    FitImageOnScreen(window, mainImage);
                    thumbnails.ChangeBrowsingListAndIndex(browsingList, browsingListIndex);
                }
            }
        }

        // Open directory
        if (gui.wantsToOpenDirectory || hotkeyShouldOpenDirectory) {
            auto selection = pfd::select_folder(
                "Select a directory",
                ".",
                {}
            );

            std::string result = selection.result();

            if (result.length() > 0) {
                OpenNewPath(window, mainImage, gui, result, gui.ignoreUnknownFileExtensions, false);
                FitImageOnScreen(window, mainImage);
                thumbnails.ChangeBrowsingListAndIndex(browsingList, browsingListIndex);
            }
        }

        // Open directory and subdirectories
        if (gui.wantsToOpenSubdirectories || hotkeyShouldOpenSubdirectories) {
            auto selection = pfd::select_folder(
                "Select a directory",
                ".",
                {}
            );

            std::string result = selection.result();

            if (result.length() > 0) {
                OpenNewPath(window, mainImage, gui, result, gui.ignoreUnknownFileExtensions, true);
                FitImageOnScreen(window, mainImage);
                thumbnails.ChangeBrowsingListAndIndex(browsingList, browsingListIndex);
            }
        }

        // Open file in explorer
        if (gui.wantsToOpenFileLocationInExplorer) {
            if (std::filesystem::exists(mainImageCurrentFilePath)) {
                std::wstring wide = mainImageCurrentFilePath.wstring();
                std::wstring command = L"explorer /select," + wide;

                _wsystem(command.c_str());
            } else {
                std::cout << "Path doesn't exist." << std::endl;
            }
        }

        // Refresh directory
        if (gui.wantsToRefreshDirectory) {
            OpenNewPath(window, mainImage, gui, mainImageCurrentFilePath, gui.ignoreUnknownFileExtensions, false);
            thumbnails.ChangeBrowsingListAndIndex(browsingList, browsingListIndex);
        }

        // Show thumbnails or not
        thumbnails.SetVisible(gui.showThumbnails);
    }

    void HandleImageShader(Window& window, Image& mainImage, GUI& gui) {
        mainImage.adjustment_ShowZebraPattern = gui.adjustment_ShowZebraPattern;
        mainImage.adjustment_ZebraPatternThreshold = gui.adjustment_ZebraPatternThreshold;

        mainImage.adjustment_NoTonemapping = gui.adjustment_NoTonemapping;
        mainImage.adjustment_UseFlatTonemapping = gui.adjustment_UseFlatTonemapping;
        mainImage.adjustment_ShowAlphaCheckerboard = gui.adjustment_ShowAlphaCheckerboard;

        mainImage.adjustment_Grayscale = gui.adjustment_Grayscale;
        mainImage.adjustment_Invert = gui.adjustment_InvertImage;
        mainImage.adjustment_Exposure = gui.adjustment_Exposure;
        mainImage.adjustment_Offset = gui.adjustment_Offset;
        mainImage.adjustment_ChannelMultiplier = gui.adjustment_rgbaChannelMultiplier;
    }

    void HandleWindowInteraction(Window& window, Image& mainImage) {
        // Fullscreen
        if (window.WasKeyFired(GLFW_KEY_F11)) {
            bool fullscreened = !window.IsFullscreen();
            window.SetFullscreen(fullscreened);

            if (fullscreened) {
                mainImagePermissibleBoundary = { 0, 0, window.GetSize().x, window.GetSize().y };
                FitImageOnScreen(window, mainImage);

                window.SetVsyncEnabled(true);
            }
        }
    }

	////////////////////////////////////////
	///// PUBLIC
	////////////////////////////////////////

    void Application::Begin(int argc, wchar_t** argv, Config config) {
        Window window({ 800, 600 }, 3, 3, 0, L"Dooky Image Viewer");
        window.SetVsyncEnabled(true);

        Image background;
        background.Create(1, 1, { 0.2f, 0.2f, 0.2f, 1.0f });

        Image informationBarBackground;
        informationBarBackground.Create(1, 1, { 0.15f, 0.15f, 0.15f, 1.0f });

        Image mainImage;
        mainImage.SetAnchorPoint(0.5f, 0.5f);
        mainImage.FlipVertically(true);
        mainImage.useMipmaps = config.useMipmaps;

        errorMessageText = new Text;
        errorMessageText->LoadFontFromPath("resources/fonts/Consolas.ttf", 16);
        errorMessageText->SetAnchorPoint(0.5f, 0.5f);

        ThumbnailPreview thumbnails;

        GUI gui;
        gui.Initialise(window.GetWindowPointer());

        Text zoomText;
        zoomText.LoadFontFromPath("resources/fonts/Consolas.ttf", 13);
        zoomText.SetColor(1.0f, 1.0f, 1.0f);
        zoomText.SetAnchorPoint(0.0f, 1.0f);

        int colorBoxSize = INFORMATION_BAR_HEIGHT - 6;

        Image colorBoxOutline;
        colorBoxOutline.Create(1, 1, { 0.85f, 0.85f, 0.85f, 1.0f });
        colorBoxOutline.SetScale(colorBoxSize + 2, colorBoxSize + 2);
        colorBoxOutline.SetAnchorPoint(0.0f, 0.5f);

        Image colorBox;
        colorBox.Create(1, 1, { 1.0f, 1.0f, 1.0f, 1.0f });
        colorBox.SetScale(colorBoxSize, colorBoxSize);
        colorBox.SetAnchorPoint(0.0f, 0.5f);

        if (argc >= 2) {
            OpenNewPath(window, mainImage, gui, argv[1], true, false);
        }

        //OpenNewPath(window, mainImage, gui, "C:\\Users\\Toby\\Desktop\\RandomImages\\20201215_203555.jpg", true, false);
        //OpenNewPath(window, mainImage, gui, "C:\\Users\\Toby\\Desktop\\RandomImages\\DotsSeparatedBy100pixels.png", true, false);

        // Set icon
        GLFWimage windowIcons[5];
        windowIcons[0].pixels = stbi_load("resources/icons/Icon_256.png", &windowIcons[0].width, &windowIcons[0].height, 0, 4);
        windowIcons[1].pixels = stbi_load("resources/icons/Icon_128.png", &windowIcons[1].width, &windowIcons[1].height, 0, 4);
        windowIcons[2].pixels = stbi_load("resources/icons/Icon_64.png", &windowIcons[2].width, &windowIcons[2].height, 0, 4);
        windowIcons[3].pixels = stbi_load("resources/icons/Icon_32.png", &windowIcons[3].width, &windowIcons[3].height, 0, 4);
        windowIcons[4].pixels = stbi_load("resources/icons/Icon_16.png", &windowIcons[4].width, &windowIcons[4].height, 0, 4);

        glfwSetWindowIcon(window.GetWindowPointer(), 5, windowIcons);

        stbi_image_free(windowIcons[0].pixels);
        stbi_image_free(windowIcons[1].pixels);
        stbi_image_free(windowIcons[2].pixels);
        stbi_image_free(windowIcons[3].pixels);
        stbi_image_free(windowIcons[4].pixels);

        mainImageEngaged = true;

        thumbnails.SetPositionAndWidth({ 0, 19 }, window.GetSize().x);
        thumbnails.ChangeBrowsingListAndIndex(browsingList, browsingListIndex);
        
		// Loop
		while (!window.ShouldClose()) {
			window.PollEventsAndUpdate();

            glm::ivec2 mousePosition = window.GetMousePosition();
            glm::ivec2 windowSize = window.GetSize();
            glm::ivec2 mainImageSize = mainImage.GetSize();

            // Hotkeys
            if (window.IsKeyDown(GLFW_KEY_LEFT_CONTROL)) {
                if (window.WasKeyFired(GLFW_KEY_O)) {
                    hotkeyShouldOpenFile = true;
                } else if (window.WasKeyFired(GLFW_KEY_P)) {
                    hotkeyShouldOpenDirectory = true;
                } else if (window.WasKeyFired(GLFW_KEY_L)) {
                    hotkeyShouldOpenSubdirectories = true;
                }
            }
            
            // Update
            HandleImageBrowsing(window, mainImage, gui);
            HandleImageInteraction(window, mainImage, thumbnails, gui);
            HandleGuiInteraction(window, mainImage, thumbnails, gui);
            HandleImageShader(window, mainImage, gui);
            HandleWindowInteraction(window, mainImage);

            UpdateMainImage(window, mainImage);

            // Thumbnails

            if (window.IsFullscreen()) {
                thumbnails.SetPositionAndWidth({ 0, 0 }, window.GetSize().x);
            } else {
                thumbnails.SetPositionAndWidth({ 0, MENU_BAR_HEIGHT }, window.GetSize().x);
            }

            thumbnails.HandleInteraction(window, gui);

            // Clicked on thumbnail so load path
            int thumbnailClickedIndex = thumbnails.GetClickedIndex();

            if (thumbnailClickedIndex >= 0 && thumbnailClickedIndex != browsingListIndex) {
                browsingListIndex = thumbnailClickedIndex;
                mainImageCurrentFilePath = browsingList[thumbnailClickedIndex];
                ChangeImage(window, mainImage, gui, mainImageCurrentFilePath);
                thumbnails.ChangeIndex(thumbnailClickedIndex);
            }
            
            // Window resized
            if (window.WasResized()) {
                thumbnails.ChangeBrowsingListAndIndex(browsingList, browsingListIndex);
                FitImageOnScreen(window, mainImage); // Fit image when resized
            }

            // If user browsed directory list then update thumbnails
            if (browsed) {
                browsed = false;
                thumbnails.ChangeIndex(browsingListIndex);
            }

            // Image boundaries
            int subtractor = window.IsFullscreen() ? MENU_BAR_HEIGHT : 0;
            int subtractor2 = gui.showInformationBar ? INFORMATION_BAR_HEIGHT : 0;

            if (gui.showThumbnails) {
                mainImagePermissibleBoundary = { 0, THUMBNAIL_PREVIEW_WITH_MENU_BAR_HEIGHT - subtractor, windowSize.x, windowSize.y - subtractor2 };
            } else {
                mainImagePermissibleBoundary = { 0, MENU_BAR_HEIGHT - subtractor, windowSize.x, windowSize.y - subtractor2 };
            }

            // Background
            background.SetScale(window.GetSize().x, window.GetSize().y);

            // Browsing list sort
            if (gui.sortByLastModifiedDate == true) {
                browsingListSortMode = 1;
            } else {
                browsingListSortMode = 0;
            }

            // Information bar
            informationBarBackground.SetScale(windowSize.x, INFORMATION_BAR_HEIGHT);
            informationBarBackground.SetAnchorPoint(0.0f, 1.0f);
            informationBarBackground.SetPosition(0.0f, windowSize.y);

            if (gui.showInformationBar) {
                std::stringstream zoomTextStr;

                // Zoom text
                bool found = false;

                for (int i = 1; i < 20; i++) {
                    if (mainImageZoom > 1.0f / powf(10.0f, i)) {
                        zoomTextStr << std::fixed << std::setprecision(i + 1) << mainImageZoom * 100 << "%";
                        found = true;
                        break;
                    }
                }

                if (!found)
                    zoomTextStr << std::fixed << std::setprecision(2) << mainImageZoom * 100 << "%";

                // Image rotation
                zoomTextStr << " | Rot:" << -mainImageRotation;

                // Selected pixel (takes into account rotation of the image)
                glm::ivec2 tempImageSize = mainImage.GetSize();
                glm::ivec2 tempMousePosition = mousePosition;

                if (mainImageRotation == -90) {
                    tempImageSize = { tempImageSize.y, tempImageSize.x };
                } else if (mainImageRotation == -180) {
                    tempMousePosition = { tempMousePosition.x, tempMousePosition.y + 1 };
                } else if (mainImageRotation == -270) {
                    tempMousePosition = { tempMousePosition.x - 1, tempMousePosition.y };
                    tempImageSize = { tempImageSize.y, tempImageSize.x };
                }

                glm::ivec2 imageTopLeft = glm::vec2(mainImagePosition) - (glm::vec2(tempImageSize) / (2.0f / mainImageZoom));
                glm::ivec2 selectedPixel = (glm::vec2(tempMousePosition - imageTopLeft) + glm::vec2(1, 1)) / mainImageZoom;

                if (mainImageRotation == -90) {
                    selectedPixel = { selectedPixel.y, mainImageSize.y - selectedPixel.x - 1 };
                } else if (mainImageRotation == -180) {
                    selectedPixel = { mainImageSize.x - selectedPixel.x - 1, mainImageSize.y - selectedPixel.y - 1 };
                } else if (mainImageRotation == -270) {
                    selectedPixel = { mainImageSize.x - selectedPixel.y - 1, selectedPixel.x };
                }

                // Clamp
                if (selectedPixel.x > mainImageSize.x - 1) {
                    selectedPixel.x = mainImageSize.x - 1;
                } else if (selectedPixel.x < 0) {
                    selectedPixel.x = 0;
                }

                if (selectedPixel.y > mainImageSize.y - 1) {
                    selectedPixel.y = mainImageSize.y - 1;
                } else if (selectedPixel.y < 0) {
                    selectedPixel.y = 0;
                }

                if (mainImageSize.x + mainImageSize.y == 0) {
                    selectedPixel = { 0, 0 };
                }

                zoomTextStr << " | X:" << std::to_string(selectedPixel.x) << " Y:" << std::to_string(selectedPixel.y);
                zoomTextStr << std::setprecision(gui.colorInfoNormalized ? 4 : 2) << " | ";

                // Selected pixel color
                glm::vec4 pixelColor = mainImage.GetPixel(selectedPixel.x, selectedPixel.y);
                int colorBoxOffset = 0;

                for (unsigned char c : zoomTextStr.str()) {
                    colorBoxOffset += zoomText.GetFont().GetCharacterAdvance(c);
                }

                int colorBoxY = windowSize.y - (INFORMATION_BAR_HEIGHT / 2) - 1; // move 1 down because needs centering, very shoddy fix

                colorBoxOutline.SetPosition(colorBoxOffset - 1, colorBoxY); // -1 because anchor point on X is 0 so to center it, it has to go to the left by 1
                colorBox.SetPosition(colorBoxOffset, colorBoxY);
                colorBox.adjustment_ChannelMultiplier = { pixelColor.r, pixelColor.g, pixelColor.b, pixelColor.a };

                int colorMult = gui.colorInfoNormalized ? 1 : 255;

                zoomTextStr << "  R:" << pixelColor.r * colorMult << " G:" << pixelColor.g * colorMult << " B:" << pixelColor.b * colorMult << " A:" << pixelColor.a * colorMult;

                // Animated image text (GIF)
                if (mainImage.GetAnimatedImageFrameCount() > 1) {
                    // Animated image progress
                    std::string frameProgressStr = std::to_string(mainImage.GetAnimatedImageCurrentIndex() + 1) + "/";
                    frameProgressStr += std::to_string(mainImage.GetAnimatedImageFrameCount());

                    std::stringstream fpsStr;
                    fpsStr << std::fixed << std::setprecision(2) << mainImage.GetAnimatedImageFPS();

                    frameProgressStr += " (" + fpsStr.str() + "fps)";

                    zoomTextStr << " | " << frameProgressStr;
                }

                // File size
                if (!fileSizeStr.str().empty()) {
                    zoomTextStr << " | " << fileSizeStr.str();
                }

                zoomText.SetPosition(mainImagePermissibleBoundary[0] + ZOOM_TEXT_OFFSET.x, window.GetSize().y + ZOOM_TEXT_OFFSET.y);
                zoomText.SetString(zoomTextStr.str());
            }

            // Save to file
            if (gui.wantsToSaveImageToFile) {
                gui.wantsToSaveImageToFile = false;

                std::string saveFileLocation = pfd::save_file(
                    "Saving to file", "", 
                    { 
                        "All Files", "*"
                    },
                    pfd::opt::none
                ).result();

                if (!saveFileLocation.empty()) {
                    bool successful = mainImage.WriteToFile(saveFileLocation);

                    if (!successful) {
                        pfd::message("Saved file as a JPEG", "File type is not supported. Saved as a JPEG instead.", pfd::choice::ok, pfd::icon::warning);
                    }
                }
            }

            // Error message text
            errorMessageText->SetPosition(mainImagePosition.x, mainImagePosition.y);

			// Render
			window.Clear();

            background.Draw(window);
			if (mainImageFailedToLoad == false) mainImage.Draw(window);
            if (gui.showInformationBar) informationBarBackground.Draw(window);
            if (gui.showInformationBar) zoomText.Draw(window);
            if (gui.showInformationBar) colorBoxOutline.Draw(window);
            if (gui.showInformationBar) colorBox.Draw(window);
            if (mainImageFailedToLoad == true) errorMessageText->Draw(window);
            thumbnails.Draw(window);
            gui.Draw(window, window.IsFullscreen());

			window.Display();

            // Reset
            hotkeyShouldOpenFile = false;
            hotkeyShouldOpenDirectory = false;
            hotkeyShouldOpenSubdirectories = false;
		}
	}
}