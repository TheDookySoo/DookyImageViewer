#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <GL/glew.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Dooky {
	class Window {
	private:
		GLFWwindow* windowPointer; // The pointer to the actual window itself
		glm::ivec2 windowSize; // The resolution of the window
		std::wstring windowTitle;

		glm::ivec2 posBeforeFullscreen;
		glm::ivec2 sizeBeforeFullscreen;

		size_t ticks; // The amount of times Display() was called
		bool isFullscreen;

		// Used for calculating delta time
		float deltaTime;
		float prevTime;
		float currentTime;

		// Flags
		bool flag_WasResized;
		bool flag_FullscreenChanged;

		// Input handling
		glm::ivec2 mouseScrollDelta;
		glm::ivec2 mouseMoveDelta;
		glm::ivec2 mousePosition;

		std::unordered_map<int, bool> keyDownMap;       // Keeps track of all the keys that are currently down
		std::unordered_map<int, bool> keyFiredMap;      // Keeps track of all the keys that were fired (useful when you want to repeat a certain function continuously if a key is held down)
		std::unordered_map<int, bool> mouseDownMap;     // Keeps track of all the mouse buttons that are down
		std::unordered_map<int, bool> mousePressedMap;  // Keeps track of all the mouse buttons that were JUST pressed (will not turn to true unless the button was released)
		std::unordered_map<int, bool> mouseReleasedMap; // Keeps track of all the mouse buttons that were JUST released, meaning it will 
		std::vector<int> textInputBuffer;               // Stores all text the user has typed, only printable characters

		std::vector<std::wstring> droppedPaths; // The paths to files/directories that was dropped onto the window
	public:
		Window(glm::ivec2 size, int glMinorVersion, int glMajorVersion, int antialiasingLevel = 0, std::wstring title = L"Window");
		~Window();

		// Important
		void PollEventsAndUpdate(); // Calls glfwPollEvents() and resets input maps
		bool ShouldClose(); // Returns true if the user wants to close the window
		void Close();

		// Input
		bool WasKeyFired(int key);          // Returns true every time a key is pressed or held (which then it will return true continuously after a short amount of time)
		bool IsKeyDown(int key);            // Returns true as long as a key is held down
		bool IsMouseButtonDown(int button); // Returns true if the mouse button specified is down
		bool WasMousePressed(int button);   // Returns true if the mouse button specified was JUST pressed
		bool WasMouseReleased(int button);  // Returns true if the mouse button specified was JUST released
		std::vector<int> GetTextInput();    // Returns a vector containing all the keys that the user has pressed
		bool WasResized();

		// Setters
		void SetTitle(const std::wstring& title);
		void SetTitle(const std::string& title);
		void SetVsyncEnabled(bool enabled);
		void SetSize(int x, int y);
		void SetFullscreen(bool enabled);

		// Getters
		GLFWwindow* GetWindowPointer();
		bool IsFullscreen();
		glm::ivec2 GetSize();
		glm::ivec2 GetPosition();
		size_t GetTicks();
		float GetTime();
		float GetDeltaTime();
		std::vector<std::wstring> GetDroppedPaths(); // The paths to the files/directories that were dragged and dropped onto the application

		glm::ivec2 GetMouseScrollDelta();
		glm::ivec2 GetMouseMoveDelta();
		glm::ivec2 GetMousePosition(); // Returns the mouse position relative to the window

		// Rendering
		void Clear();
		void Display();

		// Callbacks
		void CharCallback(GLFWwindow* window, unsigned int codepoint);
		void KeyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods);
		void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
		void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
		void WindowResizeCallback(GLFWwindow* window, int width, int height);
		void DropCallback(GLFWwindow* window, int count, const char** paths);
	};
}

#endif