#ifndef GUI_H
#define GUI_H

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_glfw.h"
#include "vendor/imgui/imgui_impl_opengl3.h"
#include "vendor/portable-file-dialogs/portable-file-dialogs.h"

#include "Window.h"

#define MENU_BAR_EXTRA_TEXT_COLOR_NORMAL { 0.5f, 0.7f, 1.0f, 1.0f };
#define MENU_BAR_EXTRA_TEXT_COLOR_ERROR { 1.0f, 0.2f, 0.2f, 1.0f };

namespace Dooky {
	class GUI {
	private:
		float lastMouseMoveTime;

		float channelRed;
		float channelGreen;
		float channelBlue;
		float channelAlpha;

		// Used for gui only
		bool showImageInformationWindow;
		bool showChannelsWindow;
		bool showZebraPatternWindow;
		bool showAdjustmentsWindow;
	public:
		ImVec4 menuBarExtraTextColor;

		std::string menuBarText;
		std::string imageInformationText;
		std::string imageInformationExifText;

		bool imguiCaptureMouse;
		bool imguiCaptureKeyboard;

		bool wantsToOpenFile;
		bool wantsToOpenDirectory;
		bool wantsToOpenSubdirectories;
		bool wantsToOpenFileLocationInExplorer;
		bool wantsToRefreshDirectory;

		bool showThumbnails;
		bool showInformationBar; // Bottom bar with zoom
		bool ignoreUnknownFileExtensions;

		bool sortByLastModifiedDate;

		// Zebra pattern

		bool adjustment_ShowZebraPattern;
		float adjustment_ZebraPatternThreshold;

		// Adjustments

		bool adjustment_Grayscale;
		bool adjustment_InvertImage;
		bool adjustment_ShowAlphaCheckerboard;
		bool adjustment_UseFlatTonemapping;
		float adjustment_Exposure;
		float adjustment_Offset;

		// Channels

		glm::vec4 adjustment_rgbaChannelMultiplier; // Used for showing individual channels of an image

		void Initialise(GLFWwindow* windowPointer);
		void Draw(Window& window, bool isFullscreened);
	};
}

#endif