#include "GUI.h"


namespace Dooky {
	////////////////////////////////////////
	///// CONSTANTS
	////////////////////////////////////////

	////////////////////////////////////////
	///// IMPORTANT
	////////////////////////////////////////

	void GUI::Initialise(GLFWwindow* windowPointer) {
		// Variable initialise

		imguiCaptureMouse = false;
		imguiCaptureKeyboard = false;

		wantsToOpenFile = false;
		wantsToOpenDirectory = false;
		wantsToOpenSubdirectories = false;
		wantsToSaveImageToFile = false;
		wantsToOpenFileLocationInExplorer = false;
		wantsToRefreshDirectory = false;

		showThumbnails = true;
		showInformationBar = true;
		ignoreUnknownFileExtensions = true;
		colorInfoNormalized = true;

		sortByLastModifiedDate = false;

		menuBarExtraTextColor = MENU_BAR_EXTRA_TEXT_COLOR_NORMAL; // Blue

		// Menus

		showImageInformationWindow = false;
		showChannelsWindow = false;
		showZebraPatternWindow = false;
		showAdjustmentsWindow = false;

		// Zebra pattern

		adjustment_ShowZebraPattern = false;
		adjustment_ZebraPatternThreshold = 0.95f;

		// Adjustments

		adjustment_Grayscale = false;
		adjustment_InvertImage = false;
		adjustment_ShowAlphaCheckerboard = true;
		adjustment_NoTonemapping = false;
		adjustment_UseFlatTonemapping = false;
		adjustment_Exposure = 0.0f;
		adjustment_Offset = 0.0f;

		// Channels

		channelRed = true;
		channelGreen = true;
		channelBlue = true;
		channelAlpha = true;
		adjustment_rgbaChannelMultiplier = { 1.0f, 1.0f, 1.0f, 1.0f };

		// Imgui initialization

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		ImGui_ImplGlfw_InitForOpenGL(windowPointer, true);
		ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));

		// Style

		ImGui::GetStyle().ChildBorderSize = 0.0f;
		ImGui::GetStyle().FrameBorderSize = 0.0f;
		ImGui::GetStyle().PopupBorderSize = 0.0f;
		ImGui::GetStyle().TabBorderSize = 0.0f;
		ImGui::GetStyle().WindowBorderSize = 0.0f;
	}

	void GUI::Draw(Window& window, bool isFullscreened) {
		// If mouse paused, don't draw

		glm::ivec2 mouseDelta = window.GetMouseMoveDelta();
		bool mouseFrozen = false;

		if (mouseDelta.x != 0 || mouseDelta.y != 0) {
			lastMouseMoveTime = window.GetTime();
		}

		if (window.GetTime() - lastMouseMoveTime > 1.0f) {
			mouseFrozen = true;
		}

		// Hide mouse when not moving and fullscreened

		if (isFullscreened && mouseFrozen) {
			glfwSetInputMode(window.GetWindowPointer(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		} else {
			glfwSetInputMode(window.GetWindowPointer(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		// Reset variables that need to be reset

		wantsToOpenFile = false;
		wantsToOpenDirectory = false;
		wantsToOpenSubdirectories = false;
		wantsToOpenFileLocationInExplorer = false;
		wantsToRefreshDirectory = false;

		// GUI

		bool shouldDraw = true;

		if (isFullscreened && mouseFrozen)
			shouldDraw = false;

		if (shouldDraw) {
			ImGuiIO& io = ImGui::GetIO();

			imguiCaptureMouse = io.WantCaptureMouse;
			imguiCaptureKeyboard = io.WantCaptureKeyboard;

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			if (ImGui::BeginMainMenuBar()) {
				// File
				if (ImGui::BeginMenu("File")) {
					if (ImGui::MenuItem("Open File")) wantsToOpenFile = true;
					if (ImGui::MenuItem("Open Directory")) wantsToOpenDirectory = true;
					if (ImGui::MenuItem("Open Subdirectories")) wantsToOpenSubdirectories = true;
					if (ImGui::MenuItem("Save To File")) wantsToSaveImageToFile = true;
					ImGui::Separator();
					if (ImGui::MenuItem("Open File In Explorer")) wantsToOpenFileLocationInExplorer = true;
					if (ImGui::MenuItem("Refresh Directory")) wantsToRefreshDirectory = true;

					ImGui::EndMenu();
				}

				// View
				if (ImGui::BeginMenu("View")) {
					ImGui::Checkbox("Thumbnails", &showThumbnails);
					ImGui::Checkbox("Show Information Bar", &showInformationBar);
					ImGui::Checkbox("Show Checkerboard", &adjustment_ShowAlphaCheckerboard);

					if (ImGui::MenuItem("Image Information")) showImageInformationWindow = true;

					ImGui::EndMenu();
				}

				// Tools
				if (ImGui::BeginMenu("Tools")) {
					if (ImGui::MenuItem("Adjustments")) showAdjustmentsWindow = true;
					if (ImGui::MenuItem("Zebra Pattern")) showZebraPatternWindow = true;

					if (ImGui::BeginMenu("Show Channels")) {
						ImGui::SetNextItemWidth(100.0f);
						ImGui::SliderFloat("Red", &channelRed, 0.0f, 1.0f, "%.4f");

						ImGui::SetNextItemWidth(100.0f);
						ImGui::SliderFloat("Green", &channelGreen, 0.0f, 1.0f, "%.4f");

						ImGui::SetNextItemWidth(100.0f);
						ImGui::SliderFloat("Blue", &channelBlue, 0.0f, 1.0f, "%.4f");

						ImGui::SetNextItemWidth(100.0f);
						ImGui::SliderFloat("Alpha", &channelAlpha, 0.0f, 1.0f, "%.4f");

						ImGui::EndMenu();
					}

					ImGui::EndMenu();
				}

				// Settings
				if (ImGui::BeginMenu("Settings")) {
					ImGui::Checkbox("Ignore Unknown Extensions", &ignoreUnknownFileExtensions);
					ImGui::Checkbox("Sort By Last Modified Time", &sortByLastModifiedDate);
					ImGui::Checkbox("Color Info Normalized", &colorInfoNormalized);

					ImGui::EndMenu();
				}

				ImGui::TextColored(menuBarExtraTextColor, menuBarText.c_str());
				ImGui::SameLine();
				ImGui::EndMainMenuBar();
			}

			// Image Information
			{
				if (showImageInformationWindow) {
					ImGui::Begin("Image Information", &showImageInformationWindow);

					ImGui::TextWrapped(imageInformationText.c_str());

					if (!imageInformationExifText.empty()) {
						ImGui::Separator();
						ImGui::TextColored({ 0.4f, 0.6f, 1.0f, 1.0f }, "EXIF INFORMATION");
						ImGui::Separator();
						ImGui::TextWrapped(imageInformationExifText.c_str());
					}

					ImGui::End();
				}
			}

			// Tools
			{
				// Zebra pattern window

				if (showZebraPatternWindow) {
					ImGui::Begin("Show Zebra Pattern", &showZebraPatternWindow);

					ImGui::Checkbox("Show Zebra Pattern", &adjustment_ShowZebraPattern);
					ImGui::SliderFloat("Threshold", &adjustment_ZebraPatternThreshold, 0.0f, 1.0f, "%.4f");

					ImGui::End();
				}

				// Adjustments window

				if (showAdjustmentsWindow) {
					ImGui::Begin("Adjustments", &showAdjustmentsWindow);

					ImGui::Checkbox("Grayscale", &adjustment_Grayscale);
					ImGui::Checkbox("Invert Image", &adjustment_InvertImage);
					ImGui::Checkbox("No Tonemapping", &adjustment_NoTonemapping);
					ImGui::Checkbox("Use Flat Tonemapping", &adjustment_UseFlatTonemapping);
					ImGui::SliderFloat("Exposure", &adjustment_Exposure, -8.0f, 8.0f, "%.2f");
					ImGui::SliderFloat("Offset", &adjustment_Offset, -1.0f, 1.0f, "%.2f");

					ImGui::End();
				}
			}

			// Update values

			adjustment_rgbaChannelMultiplier = { channelRed, channelGreen, channelBlue, channelAlpha };

			// Render

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
	}
}