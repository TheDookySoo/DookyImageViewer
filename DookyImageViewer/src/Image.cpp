#include "Image.h"

#include <iterator>
#include <fstream>
#include <unordered_set>
#include <Windows.h>

#include <Magick++.h>


std::unordered_set<std::string> TONEMAPPED_IMAGE_EXTENSIONS = {
	".hdr", ".exr", ".cr2", ".crw", ".dcr",
	".mrw", ".arw", ".nef", ".orf", ".raf",
	".x3f"
};


namespace Dooky {
	////////////////////////////////////////
	///// CLASS: IMAGE
	////////////////////////////////////////

	Image::Image() {
		//shader.LoadShaderFile("resources/shaders/Image.shader");
		shader.LoadShaderFile("C:/Users/Toby/source/repos/DookyImageViewer/DookyImageViewer/resources/shaders/Image.shader");

		animatedImagesDelaysTotal = 0;
		animatedImageHasPlayedYet = false;
		animatedImageStartPlayTime = 0;
		animatedImageIndex = 0;

		size = { 0, 0 };
		position = { 0, 0 };
		anchorPoint = { 0.0f, 0.0f };
		scale = { 1.0f, 1.0f };
		rotation = 0.0f;

		useLinearInterpolation = true;
		flipVertically = false;
		flag_ImageWasChanged = false;

		// Shader adjustments
		adjustment_UseTonemapping = false;
		adjustment_UseFlatTonemapping = false;
		adjustment_ShowAlphaCheckerboard = true;
		adjustment_ShowZebraPattern = false;
		adjustment_Grayscale = false;
		adjustment_Invert = false;
		adjustment_ZebraPatternThreshold = 0.95f;
		adjustment_Exposure = 0.0f;
		adjustment_Offset = 0.0f;

		adjustment_ChannelMultiplier = { 1.0f, 1.0f, 1.0f, 1.0f };

		vertices = {
			0.0f, 0.0f,   0.0f, 0.0f,
			1.0f, 0.0f,   1.0f, 0.0f,
			0.0f, 1.0f,   0.0f, 1.0f,
			0.0f, 1.0f,   0.0f, 1.0f,
			1.0f, 0.0f,   1.0f, 0.0f,
			1.0f, 1.0f,   1.0f, 1.0f
		};

		// Generate texture in advance
		glGenTextures(1, &textureId);

		// Generate buffers in advance (it is static so no need to change ever again)
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	Image::~Image() {
		glDeleteTextures(1, &textureId);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}

	////////////////////////////////////////
	///// PRIVATE
	////////////////////////////////////////

	void Image::Update(float* data, int w, int h) {
		glBindTexture(GL_TEXTURE_2D, textureId);

		if (useLinearInterpolation) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, data);

		//glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		// Set stuff
		size = { w, h };
	}

	void Image::GenericCreate(int w, int h, glm::vec4 c) {
		floatImageData.clear();
		floatImageData.resize(w * h * 4, 0);

		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				int i = y * w + x;

				floatImageData[i * 4 + 0] = c.r;
				floatImageData[i * 4 + 1] = c.g;
				floatImageData[i * 4 + 2] = c.b;
				floatImageData[i * 4 + 3] = c.a;
			}
		}

		Update(floatImageData.data(), w, h);
	}

	void Image::GenericSetPixel(int x, int y, glm::vec4 c) {
		int index = ((size.y - 1) - y) * size.x + x;
		
		if (index < 0 || index >= floatImageData.size() / 4)
			return;

		floatImageData[index * 4 + 0] = c.r;
		floatImageData[index * 4 + 1] = c.g;
		floatImageData[index * 4 + 2] = c.b;
		floatImageData[index * 4 + 3] = c.a;

		flag_ImageWasChanged = true; // Only update texture when drawn
	}

	////////////////////////////////////////
	///// PUBLIC
	////////////////////////////////////////

	void Image::SetPosition(int x, int y) {
		position = { x, y };
	}

	void Image::SetScale(float x, float y) {
		scale = { x, y };
	}

	void Image::SetRotation(float deg) {
		rotation = deg;
	}

	void Image::SetAnchorPoint(float x, float y) {
		anchorPoint = { x, y };
	}

	void Image::EnableLinearInterpolation(bool enabled) {
		if (enabled != useLinearInterpolation) { // Only update if we have to
			useLinearInterpolation = enabled;

			glBindTexture(GL_TEXTURE_2D, textureId);

			if (enabled) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			} else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	void Image::FlipVertically(bool enabled) {
		flipVertically = enabled;
	}

	void Image::SetPixel(int x, int y, glm::vec3 c) {
		GenericSetPixel(x, y, { c.r, c.g, c.b, 1.0f });
	}

	void Image::SetPixel(int x, int y, glm::vec4 c) {
		GenericSetPixel(x, y, { c.r, c.g, c.b, c.a });
	}

	void Image::SetPixel(int x, int y, float r, float g, float b) {
		GenericSetPixel(x, y, { r, g, b, 1.0f });
	}

	void Image::SetPixel(int x, int y, float r, float g, float b, float a) {
		GenericSetPixel(x, y, { r, g, b, a });
	}

	glm::vec4 Image::GetPixel(int x, int y) {
		size_t index = y * size.x + x;
		
		if (index * 4 + 3 >= 0 && index + 4 + 3 < floatImageData.size()) {
			if (floatImageData.empty())
				return { 0, 0, 0, 0 };
			
			return {
				floatImageData[index * 4 + 0],
				floatImageData[index * 4 + 1],
				floatImageData[index * 4 + 2],
				floatImageData[index * 4 + 3]
			};
		}
	}

	glm::ivec2 Image::GetSize() {
		return size;
	}

	glm::ivec2 Image::GetPosition() {
		return position;
	}

	glm::vec2 Image::GetAnchorPoint() {
		return anchorPoint;
	}

	glm::vec2 Image::GetScale() {
		return scale;
	}

	int Image::GetAnimatedImageCurrentIndex() {
		return animatedImageIndex;
	}

	int Image::GetAnimatedImageFrameCount() {
		return animatedImages.size();
	}
	
	float Image::GetAnimatedImageFPS() {
		return animatedImageFPS;
	}

	void Image::Create(int w, int h, glm::vec3 c) {
		GenericCreate(w, h, { c.r, c.g, c.b, 1.0f });
	}

	void Image::Create(int w, int h, glm::vec4 c) {
		GenericCreate(w, h, c);
	}

	void Image::Create(int w, int h, float r, float g, float b) {
		GenericCreate(w, h, { r, g, b, 1.0f });
	}

	void Image::Create(int w, int h, float r, float g, float b, float a) {
		GenericCreate(w, h, { r, g, b, a });
	}

	void Image::LoadRawData(int width, int height, std::vector<unsigned char> data) {
		size = { width, height };
		floatImageData.resize(data.size(), 0);
		
		for (size_t i = 0; i < data.size(); i++) {
			floatImageData[i] = (float)data[i] / 255;
		}

		Update(floatImageData.data(), width, height);
	}

	bool Image::LoadImageFile(const std::filesystem::path& path) {
		if (!std::filesystem::exists(path))
			return false;

		// wstring to utf8
		char convertedPath[1024];
		WideCharToMultiByte(65001, 0, path.wstring().c_str(), -1, convertedPath, 1024, NULL, NULL);

		std::string extension = path.extension().string();

		std::transform(
			extension.begin(),
			extension.end(),
			extension.begin(),
			[](unsigned char c) {
				return std::tolower(c);
			}
		);

		// Load
		int width = 0;
		int height = 0;

		try {
			std::list<Magick::Image> imageList;
			Magick::readImages(&imageList, convertedPath);

			if (imageList.empty())
				false;

			// Clear animated images
			animatedImages.clear();
			animatedImagesDelays.clear();
			animatedImagesDelaysTotal = 0;
			animatedImageHasPlayedYet = false;

			// Decide if image should be tonemapped
			adjustment_UseTonemapping = TONEMAPPED_IMAGE_EXTENSIONS.contains(extension);

			Magick::Image* frontImage = &imageList.front();
			width = frontImage->size().width();
			height = frontImage->size().height();

			if (imageList.size() == 1) { // Single, static image
				frontImage->type(Magick::TrueColorAlphaType);

				// Append data
				float* data = frontImage->getPixels(0, 0, width, height);
				floatImageData.resize(width * height * 4, 0);
				floatImageData.assign(data, data + (width * height * 4));

				// Divide by 65535 to reduce intensity for shader
				for (float& f : floatImageData) f /= 65535;

				Update(floatImageData.data(), width, height);
			} else if (imageList.size() > 1) {
				Magick::coalesceImages(&imageList, imageList.begin(), imageList.end()); // For when GIFs have page offsets

				int frameIndex = 0;

				// Animated image, probably GIF
				for (auto& image : imageList) {
					image.type(Magick::TrueColorAlphaType);

					int columns = image.columns();
					int rows = image.rows();
					float* data = image.getPixels(0, 0, columns, rows);

					AnimatedImageFrame frame;
					frame.index = frameIndex;
					frame.data.resize(width * height * 4, 0);
					frame.data.assign(data, data + (columns * rows * 4));

					int delay = image.animationDelay();

					// Divide by 65535 to reduce intensity for shader
					for (float& f : frame.data) f /= 65535;
					
					animatedImages.insert(std::pair<int, AnimatedImageFrame>(animatedImagesDelaysTotal, frame));
					animatedImagesDelays.push_back(delay);
					animatedImagesDelaysTotal += delay;
					frameIndex++;
				}

				animatedImageFPS = (float)(animatedImages.size() * 100) / animatedImagesDelaysTotal;

				// Update with first frame
				auto got = animatedImages.find(0);

				if (got != animatedImages.end()) {
					floatImageData = animatedImages.find(0)->second.data;
				}

				Update(floatImageData.data(), width, height);
			}

			size = { width, height };

			return true;
		} catch (std::exception& exception) {
			//std::cout << "LOAD IMAGE EXCEPTION: " << exception.what() << std::endl;
			return false;
		}
		
		return true;
	}

	void Image::Draw(Window& window) {
		if (flag_ImageWasChanged) {
			flag_ImageWasChanged = false;
			Update(floatImageData.data(), size.x, size.y);
		}

		// Animated image updates, e.g animated GIF
		if (animatedImages.size() > 1 && animatedImagesDelaysTotal > 0) {
			if (animatedImageHasPlayedYet == false) {
				animatedImageHasPlayedYet = true;
				animatedImageStartPlayTime = window.GetTime();
			}

			int index = (int)((window.GetTime() - animatedImageStartPlayTime) * 100) % animatedImagesDelaysTotal;

			auto got = animatedImages.find(index);

			if (got != animatedImages.end()) {
				auto& data = got->second.data;
				animatedImageIndex = got->second.index;

				for (int i = 0; i < data.size() / 4; i++) {
					float r = data[i * 4 + 0];
					float g = data[i * 4 + 1];
					float b = data[i * 4 + 2];
					float a = data[i * 4 + 3];

					if (a > 0) {
						floatImageData[i * 4 + 0] = data[i * 4 + 0];
						floatImageData[i * 4 + 1] = data[i * 4 + 1];
						floatImageData[i * 4 + 2] = data[i * 4 + 2];
						floatImageData[i * 4 + 3] = data[i * 4 + 3];
					}
				}

				Update(floatImageData.data(), size.x, size.y);
			}
		}

		glm::ivec2 winSize = window.GetSize();
		float winWidth = winSize.x;
		float winHeight = winSize.y;

		glm::mat4 projection = glm::ortho(0.0f, winWidth, 0.0f, winHeight);
		projection = glm::translate(projection, { position.x, winHeight - position.y, 0.0f }); // Move to position
		projection = glm::rotate(projection, glm::radians(rotation), { 0.0f, 0.0f, 1.0f }); // Rotate
		projection = glm::translate(projection, { floorf(-size.x * scale.x * anchorPoint.x), floorf(size.y * scale.y * anchorPoint.y), 0.0f }); // Move down to correct pivot point
		projection = glm::translate(projection, { 0.0f, -size.y * scale.y, 0.0f }); // Move down one last time

		shader.Bind();

		shader.SetUniformMat4fv("projection", projection);
		shader.SetUniform2f("size", size.x, size.y);
		shader.SetUniform2f("scale", scale.x, scale.y);
		shader.SetUniformBool("flipVertically", flipVertically);

		shader.SetUniform1f("time", window.GetTime());
		shader.SetUniform2f("position", position.x, position.y);

		shader.SetUniformBool("adjustment_UseTonemapping"        , adjustment_UseTonemapping);
		shader.SetUniformBool("adjustment_UseFlatTonemapping"    , adjustment_UseFlatTonemapping);
		shader.SetUniformBool("adjustment_ShowAlphaCheckerboard" , adjustment_ShowAlphaCheckerboard);
		shader.SetUniformBool("adjustment_ShowZebraPattern"      , adjustment_ShowZebraPattern);
		shader.SetUniformBool("adjustment_Grayscale"             , adjustment_Grayscale);
		shader.SetUniformBool("adjustment_Invert"                , adjustment_Invert);
		shader.SetUniform1f("adjustment_ZebraPatternThreshold"   , adjustment_ZebraPatternThreshold);
		shader.SetUniform1f("adjustment_Exposure"                , adjustment_Exposure);
		shader.SetUniform1f("adjustment_Offset"                  , adjustment_Offset);

		auto m = adjustment_ChannelMultiplier;
		shader.SetUniform4f("adjustment_ChannelMultiplier", m.r, m.g, m.b, m.a);

		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);

		shader.Unbind();
	}
}