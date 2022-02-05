#ifndef FONT_H
#define FONT_H

#include <filesystem>
#include <array>
#include <glm/glm.hpp>

namespace Dooky {
	struct Glyph {
		glm::ivec2 size;
		glm::ivec2 bearing;
		glm::vec4 texCoords; // Ranges from 0.0f to 1.0f

		int advanceInPixels;
	};

	class Font {
	private:
		int fontSize;
		int lineHeight;
		unsigned int bitmapTextureId;
		glm::ivec2 bitmapSize;
		std::filesystem::path fontPath;

		std::vector<unsigned char> bitmap;
		std::array<Glyph, 128> glyphs;

		unsigned char MakeNonAsciiSquare(unsigned char c);
	public:
		Font();

		int GetFontSize();
		int GetLineHeight();
		int GetCharacterAdvance(unsigned char c);
		unsigned int GetBitmapTextureId();
		unsigned char* GetBitmap();
		glm::ivec2 GetBitmapSize();
		std::array<Glyph, 128>& GetGlyphsArray();
		void SetFontSize(int newFontSize);

		void LoadFontFromPath(const std::filesystem::path& path, int fontSize);
	};
}

#endif