#include "Font.h"

#include <GL/glew.h>
#include <Windows.h>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Dooky {
	Font::Font() {
		fontSize = 0;
		lineHeight = 0;
		bitmapSize = { 0, 0 };
		bitmapTextureId = 0;
	}

	unsigned char Font::MakeNonAsciiSquare(unsigned char c) {
		if (c > 127) return 1;
		return c;
	}

	int Font::GetFontSize() {
		return fontSize;
	}

	int Font::GetLineHeight() {
		return lineHeight;
	}

	int Font::GetCharacterAdvance(unsigned char c) {
		assert(c < 128);

		return glyphs[c].advanceInPixels;
	}

	unsigned int Font::GetBitmapTextureId() {
		return bitmapTextureId;
	}

	unsigned char* Font::GetBitmap() {
		return bitmap.data();
	}

	glm::ivec2 Font::GetBitmapSize() {
		return bitmapSize;
	}

	std::array<Glyph, 128>& Font::GetGlyphsArray() {
		return glyphs;
	}

	void Font::SetFontSize(int newFontSize) {
		LoadFontFromPath(fontPath, newFontSize);
	}

	void Font::LoadFontFromPath(const std::filesystem::path& path, int fontSize) {
		assert(std::filesystem::is_regular_file(path));

		// wstring to utf8
		char convertedPath[1024];
		WideCharToMultiByte(65001, 0, path.wstring().c_str(), -1, convertedPath, 1024, NULL, NULL);

		bitmap.clear();
		bitmapSize = { 0, 0 };
		this->fontSize = fontSize;
		lineHeight = 0;
		fontPath = path;

		// Load font
		FT_Library library;
		FT_Init_FreeType(&library);

		FT_Face face;
		FT_New_Face(library, convertedPath, 0, &face);
		FT_Set_Pixel_Sizes(face, 0, fontSize);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Padding
		glm::ivec2 padding = { 4, 4 }; // Prevents weird edge forming around characters when rotated

		// Calculate bounds of bitmap image
		for (unsigned char c = 0; c < 128; c++) {
			FT_Load_Char(face, c, FT_LOAD_RENDER);

			int glyphWidth = face->glyph->bitmap.width + padding.x;
			int glyphHeight = face->glyph->bitmap.rows + padding.y;

			bitmapSize.x += glyphWidth;
			bitmapSize.y = bitmapSize.y > glyphHeight ? bitmapSize.y : glyphHeight;
		}

		bitmap.resize(bitmapSize.x * bitmapSize.y, 0); // Fill image with black

		// Create a glyph for every character
		int cursorX = 0;

		for (unsigned char c = 0; c < 128; c++) {
			FT_Load_Char(face, c, FT_LOAD_RENDER);

			int glyphWidth = face->glyph->bitmap.width;
			int glyphHeight = face->glyph->bitmap.rows;
			unsigned char* buffer = face->glyph->bitmap.buffer;

			Glyph glyph;
			glyph.size = { glyphWidth, glyphHeight };
			glyph.bearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };
			glyph.advanceInPixels = face->glyph->advance.x >> 6;
			glyph.texCoords = { (float)cursorX / bitmapSize.x, (padding.y / 2.0f) / bitmapSize.y, (float)glyphWidth / bitmapSize.x, (float)glyphHeight / bitmapSize.y };

			if (c == '	') // <control> aka TAB is 4 spaces
				glyph.advanceInPixels *= 4;

			glyphs[c] = glyph;

			// Fill in 
			for (int y = 0; y < glyphHeight; y++) {
				for (int x = 0; x < glyphWidth; x++) {
					int globalIndex = (y + padding.y / 2) * bitmapSize.x + (cursorX + x);
					int localIndex = y * glyphWidth + x;

					bitmap[globalIndex] = buffer[localIndex];
				}
			}

			cursorX += glyphWidth + padding.x;
			lineHeight = fmax(lineHeight, glyph.size.y);
		}

		// Finish
		FT_Done_Face(face);
		FT_Done_FreeType(library);

		// Generate texture
		glGenTextures(1, &bitmapTextureId);
		glBindTexture(GL_TEXTURE_2D, bitmapTextureId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bitmapSize.x, bitmapSize.y, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.data());
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}