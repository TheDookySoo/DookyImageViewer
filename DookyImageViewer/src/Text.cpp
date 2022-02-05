#include "Text.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Dooky {
	Text::Text() {
		alignment = TextAlignment::Left;
		bounds = { 0, 0 };
		position = { 0, 0 };
		anchorPoint = { 0.0f, 0.0f, };
		color = { 1.0f, 1.0f, 1.0f, 1.0f };
		rotation = 0.0f;

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);

		shader.LoadShaderFile("resources/shaders/Text.shader");
	}

	Text::~Text() {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}

	void Text::UpdateText() {
		vertices.clear();
		bounds = { 0, 0 };

		auto glyphs = font.GetGlyphsArray();
		int fontSize = font.GetFontSize();
		int lineHeight = font.GetLineHeight();

		// Calculate offsets for text alignment
		std::vector<float> alignmentOffsets;
		alignmentOffsets.push_back(0.0f);

		int currentLine = 0;

		for (unsigned char c : string) {
			c = MakeNonAsciiSquare(c);

			if (c == '\n') {
				currentLine++;
				alignmentOffsets.push_back(0.0f);

				continue;
			}

			if (alignment == TextAlignment::Center) {
				alignmentOffsets[currentLine] -= (float)font.GetCharacterAdvance(c) / 2.0f;
			}
			else if (alignment == TextAlignment::Right) {
				alignmentOffsets[currentLine] -= (float)font.GetCharacterAdvance(c);
			}
		}

		// Reserve in advance
		vertices.reserve(24 * string.length());

		// Loop through every character in the string
		int cursorX = alignmentOffsets[0];
		int cursorY = 0;
		currentLine = 0;

		for (unsigned char c : string) {
			auto glyph = glyphs.at(c);

			// If character is newline, skip to next line and do not render
			if (c == '\n') {
				currentLine++;

				cursorX = alignmentOffsets[currentLine];
				cursorY -= lineHeight;

				continue;
			}

			// If character is space or tab, skip to next line and do not render
			if (c == ' ' || c == '	') {
				cursorX += glyph.advanceInPixels;
				continue;
			}

			float w = glyph.size.x;
			float h = glyph.size.y;
			float x = cursorX + glyph.bearing.x;
			float y = cursorY - (h - glyph.bearing.y);

			float tex_X = glyph.texCoords[0];
			float tex_Y = glyph.texCoords[1];
			float tex_Width = glyph.texCoords[2];
			float tex_Height = glyph.texCoords[3];

			float glyphVertices[24] = {
				x    , y + h, tex_X            , tex_Y             ,
				x    , y    , tex_X            , tex_Y + tex_Height,
				x + w, y    , tex_X + tex_Width, tex_Y + tex_Height,
				x    , y + h, tex_X            , tex_Y             ,
				x + w, y    , tex_X + tex_Width, tex_Y + tex_Height,
				x + w, y + h, tex_X + tex_Width, tex_Y
			};

			// Calculate bounds
			//bounds = { fmax(bounds.x, x + w), fmax(bounds.y, y + h) };
			bounds = { fmax(bounds.x, x + w), bounds.y };

			// Append vertices to vector
			for (int i = 0; i < 24; i++)
				vertices.push_back(glyphVertices[i]);

			// Advance cursor to the right
			cursorX += glyph.advanceInPixels;
		}

		bounds.y = currentLine * lineHeight;

		// Update buffer data
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	unsigned char Text::MakeNonAsciiSquare(unsigned char c) {
		if (c > 127) return 1;
		return c;
	}

	void Text::LoadFontFromPath(const std::string& path, int fontSize) {
		font.LoadFontFromPath(path, fontSize);
	}

	void Text::LoadShader(const std::string& path) {
		shader.LoadShaderFile(path);
	}

	void Text::SetString(const std::string& newString) {
		if (string != newString) {
			string = newString;
			UpdateText();
		}
	}

	void Text::SetPosition(int x, int y) {
		position = { x, y };
	}

	void Text::SetRotation(float deg) {
		rotation = deg;
	}

	void Text::SetAnchorPoint(float x, float y) {
		anchorPoint = { x, y };
	}

	void Text::SetColor(float r, float g, float b) {
		color = { r, g, b, 1.0f };
	}

	void Text::SetColor(float r, float g, float b, float a) {
		color = { r, g, b, a };
	}

	void Text::SetColor(glm::vec3 color) {
		this->color = { color.r, color.g, color.b, 1.0f };
	}

	void Text::SetColor(glm::vec4 color) {
		this->color = { color.r, color.g, color.b, color.a };
	}

	void Text::SetHorizontalAlignment(TextAlignment alignment) {
		this->alignment = alignment;
	}

	Font& Text::GetFont() {
		return font;
	}

	void Text::Draw(Window& window) {
		glm::ivec2 winSize = window.GetSize();
		float winWidth = winSize.x;
		float winHeight = winSize.y;
		int fontSize = font.GetFontSize();
		int lineHeight = font.GetLineHeight();

		glm::mat4 projection = glm::ortho(0.0f, winWidth, 0.0f, winHeight);
		projection = glm::translate(projection, { position.x, winHeight - position.y, 0.0f }); // Move to position
		projection = glm::rotate(projection, glm::radians(rotation), { 0.0f, 0.0f, 1.0f }); // Rotate
		projection = glm::translate(projection, { floorf(-bounds.x * anchorPoint.x), floorf(((bounds.y + lineHeight) * anchorPoint.y) - lineHeight), 0.0f }); // Move down to correct pivot point


		shader.Bind();
		shader.SetUniformMat4fv("projection", projection);
		shader.SetUniform4f("textColor", color.r, color.g, color.b, color.a);

		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, font.GetBitmapTextureId());
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 4);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);

		shader.Unbind();
	}
}