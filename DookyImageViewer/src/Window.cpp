#include "Window.h"

#include <Windows.h>

namespace Dooky {
	////////////////////////////////////////
	///// PUBLIC
	////////////////////////////////////////

	Window::Window(glm::ivec2 size, int glMinorVersion, int glMajorVersion, int antialiasingLevel, std::wstring title) {
		// Set some variables
		windowTitle = title;
		windowSize = size;

		mouseScrollDelta = { 0, 0 };
		mouseMoveDelta = { 0, 0 };
		mousePosition = { 0, 0 };
		deltaTime = 0.0f;
		prevTime = 0.0f;
		currentTime = 0.0f;
		ticks = 0;
		flag_WasResized = false;
		flag_FullscreenChanged = false;
		isFullscreen = false;

		posBeforeFullscreen = windowSize;
		sizeBeforeFullscreen = { 0, 0 };

		// Initialize GLEW
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glMajorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glMinorVersion);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // No need for deprecated functions

		glfwWindowHint(GLFW_SAMPLES, antialiasingLevel);

		// Create window
		windowPointer = glfwCreateWindow(size.x, size.y, "", NULL, NULL);
		glfwMakeContextCurrent(windowPointer);

		// Set title
		HWND win32handle = glfwGetWin32Window(windowPointer);
		SetWindowText(win32handle, title.c_str());

		// Set callbacks
		glfwSetWindowUserPointer(windowPointer, this);

		auto charCallback = [](GLFWwindow* w, unsigned int codepoint) { static_cast<Window*>(glfwGetWindowUserPointer(w))->CharCallback(w, codepoint); };
		auto keyCallback = [](GLFWwindow* w, int key, int scanCode, int action, int mods) { static_cast<Window*>(glfwGetWindowUserPointer(w))->KeyCallback(w, key, scanCode, action, mods); };
		auto scrollCallback = [](GLFWwindow* w, double xOffset, double yOffset) { static_cast<Window*>(glfwGetWindowUserPointer(w))->ScrollCallback(w, xOffset, yOffset); };
		auto windowResizeCallback = [](GLFWwindow* w, int width, int height) { static_cast<Window*>(glfwGetWindowUserPointer(w))->WindowResizeCallback(w, width, height); };
		auto mouseButtonCallback = [](GLFWwindow* w, int button, int action, int mods) { static_cast<Window*>(glfwGetWindowUserPointer(w))->MouseButtonCallback(w, button, action, mods); };
		auto cursorPositionCallback = [](GLFWwindow* w, double xpos, double ypos) { static_cast<Window*>(glfwGetWindowUserPointer(w))->CursorPositionCallback(w, xpos, ypos); };
		auto droppedCallback = [](GLFWwindow* w, int count, const char** paths) { static_cast<Window*>(glfwGetWindowUserPointer(w))->DropCallback(w, count, paths); };

		glfwSetCharCallback(windowPointer, charCallback);
		glfwSetKeyCallback(windowPointer, keyCallback);
		glfwSetScrollCallback(windowPointer, scrollCallback);
		glfwSetWindowSizeCallback(windowPointer, windowResizeCallback);
		glfwSetMouseButtonCallback(windowPointer, mouseButtonCallback);
		glfwSetCursorPosCallback(windowPointer, cursorPositionCallback);
		glfwSetDropCallback(windowPointer, droppedCallback);

		// Initialize GLEW
		glewInit();

		// Enable transparency/blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	Window::~Window() {
		glfwDestroyWindow(windowPointer);
	}

	////////////////////////////////////////
	// IMPORTANT
	////////////////////////////////////////

	void Window::PollEventsAndUpdate() {
		// Reset
		mouseScrollDelta = { 0, 0 };
		mouseMoveDelta = { 0, 0 };

		keyFiredMap.clear();
		mousePressedMap.clear();
		mouseReleasedMap.clear();

		// If fullscreen is updated then WasResized needs to be retained for another frame as flag_FullscreenChanged will be immediately set to false so it acts as a buffer
		if (flag_FullscreenChanged) {
			flag_FullscreenChanged = false;
		} else {
			flag_WasResized = false;
		}

		// Time
		prevTime = currentTime;
		currentTime = glfwGetTime();
		deltaTime = currentTime - prevTime;

		// Poll events
		glfwPollEvents();
	}

	bool Window::ShouldClose() {
		return glfwWindowShouldClose(windowPointer);
	}

	void Window::Close() {
		glfwDestroyWindow(windowPointer);
		glfwTerminate();
	}

	////////////////////////////////////////
	// INPUT
	////////////////////////////////////////

	bool Window::WasKeyFired(int key) {
		if (keyFiredMap.contains(key))
			if (keyFiredMap[key] == true)
				return true;

		return false;
	}

	bool Window::IsKeyDown(int key) {
		if (keyDownMap.contains(key))
			if (keyDownMap[key] == true)
				return true;

		return false;
	}

	bool Window::IsMouseButtonDown(int button) {
		if (mouseDownMap.contains(button))
			if (mouseDownMap[button] == true)
				return true;

		return false;
	}

	bool Window::WasMousePressed(int button) {
		if (mousePressedMap.contains(button))
			if (mousePressedMap[button] == true)
				return true;

		return false;
	}

	bool Window::WasMouseReleased(int button) {
		if (mouseReleasedMap.contains(button))
			if (mouseReleasedMap[button] == true)
				return true;

		return false;
	}

	std::vector<int> Window::GetTextInput() {
		auto buffer = textInputBuffer;
		textInputBuffer.clear();

		return buffer;
	}

	bool Window::WasResized() {
		return flag_WasResized;
	}

	glm::ivec2 Window::GetMouseScrollDelta() {
		return mouseScrollDelta;
	}

	glm::ivec2 Window::GetMouseMoveDelta() {
		return mouseMoveDelta;
	}

	glm::ivec2 Window::GetMousePosition() {
		return mousePosition;
	}

	////////////////////////////////////////
	// SETTERS
	////////////////////////////////////////

	void Window::SetTitle(const std::wstring& title) {
		HWND win32handle = glfwGetWin32Window(windowPointer);
		SetWindowText(win32handle, title.c_str());
	}

	void Window::SetTitle(const std::string& title) {
		glfwSetWindowTitle(windowPointer, title.c_str());
	}

	void Window::SetVsyncEnabled(bool enabled) {
		if (enabled) {
			glfwSwapInterval(1);
		}
		else {
			glfwSwapInterval(0);
		}
	}

	void Window::SetSize(int x, int y) {
		windowSize = { x, y };
		glfwSetWindowSize(windowPointer, x, y);
	}

	void Window::SetFullscreen(bool enabled) {
		if (enabled) {
			glm::ivec2 windowPosition = GetPosition();
			glm::ivec2 windowCenter = windowPosition + (windowSize / 2);

			int monitorCount;
			float minDistance = 0.0f;
			GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);

			int closestMonitorIndex = -1;
			int closestMonitorRefreshRate;
			glm::ivec2 closestMonitorSize;

			// Determine closest monitor
			for (int i = 0; i < monitorCount; i++) {
				int xpos, ypos;
				auto monitor = monitors[i];
				glfwGetMonitorPos(monitor, &xpos, &ypos);

				auto mode = glfwGetVideoMode(monitor);
				glm::ivec2 monitorCenter = { xpos + (mode->width / 2), ypos + (mode->height / 2) };
				glm::vec2 difference = monitorCenter - windowCenter;

				float distance = sqrtf(difference.x * difference.x + difference.y * difference.y);

				if (distance < minDistance || i == 0) {
					minDistance = distance;

					closestMonitorIndex = i;
					closestMonitorRefreshRate = mode->refreshRate;
					closestMonitorSize = { mode->width, mode->height };
				}
			}

			// Fullscreen to that monitor
			if (monitorCount > 0 && closestMonitorIndex >= 0) {
				posBeforeFullscreen = windowPosition;
				sizeBeforeFullscreen = windowSize;
				glfwSetWindowMonitor(windowPointer, monitors[closestMonitorIndex], 0, 0, closestMonitorSize.x, closestMonitorSize.y, closestMonitorRefreshRate);
			}
		} else {
			int xpos = posBeforeFullscreen.x;
			int ypos = posBeforeFullscreen.y;
			int width = sizeBeforeFullscreen.x;
			int height = sizeBeforeFullscreen.y;

			glfwSetWindowMonitor(windowPointer, nullptr, xpos, ypos, width, height, 0);
		}

		flag_FullscreenChanged = true;
		isFullscreen = enabled;
	}

	////////////////////////////////////////
	// GETTERS
	////////////////////////////////////////

	GLFWwindow* Window::GetWindowPointer() {
		return windowPointer;
	}

	bool Window::IsFullscreen() {
		return isFullscreen;
	}

	glm::ivec2 Window::GetSize() {
		return windowSize;
	}

	glm::ivec2 Window::GetPosition() {
		int x = 0;
		int y = 0;

		glfwGetWindowPos(windowPointer, &x, &y);

		return { x, y };
	}

	size_t Window::GetTicks() {
		return ticks;
	}

	float Window::GetTime() {
		return glfwGetTime();
	}

	float Window::GetDeltaTime() {
		return deltaTime;
	}

	std::vector<std::wstring> Window::GetDroppedPaths() {
		std::vector<std::wstring> copy = droppedPaths;
		droppedPaths.clear();

		return copy;
	}

	////////////////////////////////////////
	// RENDERING
	////////////////////////////////////////

	void Window::Clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Window::Display() {
		ticks++;
		glfwSwapBuffers(windowPointer);
	}

	////////////////////////////////////////
	// CALLBACKS
	////////////////////////////////////////

	void Window::CharCallback(GLFWwindow* window, unsigned int codepoint) {
		textInputBuffer.push_back((char)codepoint);
	}

	void Window::KeyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods) {
		// Action:
		// 0 - release
		// 1 - press
		// 2 - hold

		if (action == 0) {
			keyDownMap[key] = false;
		} else {
			keyDownMap[key] = true;
			keyFiredMap[key] = true;
		}
	}

	void Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		// Action:
		// 0 - release
		// 1 - press

		if (action == 0) {
			mouseDownMap[button] = false;
			mouseReleasedMap[button] = true;
		} else {
			mouseDownMap[button] = true;
			mousePressedMap[button] = true;
		}
	}

	void Window::ScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
		mouseScrollDelta = { xOffset, yOffset };
	}

	void Window::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
		glm::ivec2 position = { xpos, ypos };

		mouseMoveDelta = mousePosition - position;
		mousePosition = position;
	}

	void Window::WindowResizeCallback(GLFWwindow* window, int width, int height) {
		flag_WasResized = true;

		windowSize = { width, height };
		glViewport(0, 0, width, height);
	}

	void Window::DropCallback(GLFWwindow* window, int count, const char** paths) {
		droppedPaths.clear();

		for (int i = 0; i < count; i++) {
			std::string str = paths[i];

			int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str.at(0), (int)str.size(), NULL, 0);
			std::wstring wStr(sizeNeeded, 0);
			MultiByteToWideChar(CP_UTF8, 0, &str.at(0), (int)str.size(), &wStr.at(0), sizeNeeded);

			droppedPaths.push_back(wStr);
		}
	}
}