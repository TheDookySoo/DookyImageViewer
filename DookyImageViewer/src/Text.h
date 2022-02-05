#ifndef TEXT_H
#define TEXT_H

#include <GL/glew.h>

#include <string>
#include "Window.h"
#include "Font.h"
#include "Shader.h"

namespace Dooky {
	enum class TextAlignment {
		Left,
		Center,
		Right
	};

	class Text {
	private:
		unsigned int vao;
		unsigned int vbo;

		Font font;
		Shader shader;

		std::vector<float> vertices;
		std::string fontPath;
		std::string string;
		glm::ivec2 bounds;
		glm::ivec2 position;
		glm::vec2 anchorPoint;
		glm::vec4 color;
		float rotation;

		TextAlignment alignment;

		void UpdateText();
		unsigned char MakeNonAsciiSquare(unsigned char c);
	public:
		Text();
		~Text();

		void LoadFontFromPath(const std::string& path, int fontSize);
		void LoadShader(const std::string& path); // Overrides default shader

		void SetString(const std::string& newString);
		void SetPosition(int x, int y);
		void SetRotation(float deg);
		void SetAnchorPoint(float x, float y);
		void SetColor(float r, float g, float b);
		void SetColor(float r, float g, float b, float a);
		void SetColor(glm::vec3 color);
		void SetColor(glm::vec4 color);
		void SetHorizontalAlignment(TextAlignment alignment);

		Font& GetFont();

		void Draw(Window& window);
	};
}

#endif