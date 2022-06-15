#ifndef IMAGE_H
#define IMAGE_H

#include <vector>
#include <filesystem>
#include <unordered_map>
#include <glm/glm.hpp>

#include "Window.h"
#include "Shader.h"

namespace Dooky {
	struct AnimatedImageFrame {
		std::vector<float> data;
		int index;
	};

	class Image {
	private:
		std::unordered_map<int, AnimatedImageFrame> animatedImages;
		std::vector<int> animatedImagesDelays;
		int animatedImagesDelaysTotal;
		bool animatedImageHasPlayedYet;
		float animatedImageStartPlayTime;
		float animatedImageFPS;
		int animatedImageIndex;

		std::vector<float> vertices;
		std::vector<float> floatImageData;
		glm::ivec2 size; // The resolution of the image
		glm::ivec2 position;
		glm::vec2 anchorPoint;
		glm::vec2 scale;
		float rotation;

		bool useLinearInterpolation;
		bool flipVertically;

		bool flag_ImageWasChanged;

		unsigned int vao;
		unsigned int vbo;
		unsigned int textureId;

		Shader shader;

		void Update(float* data, int w, int h);
		void GenericCreate(int w, int h, glm::vec4 c);
		void GenericSetPixel(int x, int y, glm::vec4 c);
	public:
		bool useTonemapping;
		bool useMipmaps;

		bool adjustment_NoTonemapping;
		bool adjustment_UseFlatTonemapping;
		bool adjustment_ShowAlphaCheckerboard;
		bool adjustment_ShowZebraPattern;
		bool adjustment_Invert;
		bool adjustment_Grayscale;
		float adjustment_ZebraPatternThreshold;
		float adjustment_Exposure;
		float adjustment_Offset;
		glm::vec4 adjustment_ChannelMultiplier;
		
		bool fixasdf;

		Image();
		~Image();

		void SetPosition(int x, int y);
		void SetScale(float x, float y);
		void SetRotation(float deg);
		void SetAnchorPoint(float x, float y);
		void EnableLinearInterpolation(bool enabled);
		void FlipVertically(bool enabled);

		void SetPixel(int x, int y, glm::vec3 c);
		void SetPixel(int x, int y, glm::vec4 c);
		void SetPixel(int x, int y, float r, float g, float b);
		void SetPixel(int x, int y, float r, float g, float b, float a);

		glm::vec4 GetPixel(int x, int y);
		glm::ivec2 GetSize();
		glm::ivec2 GetPosition();
		glm::vec2 GetAnchorPoint();
		glm::vec2 GetScale();
		int GetAnimatedImageCurrentIndex();
		int GetAnimatedImageFrameCount();
		float GetAnimatedImageFPS();
		float* GetRawImageData();

		void Create(int w, int h, glm::vec3 c);
		void Create(int w, int h, glm::vec4 c);
		void Create(int w, int h, float r, float g, float b);
		void Create(int w, int h, float r, float g, float b, float a);

		void LoadRawData(int width, int height, std::vector<unsigned char> data);
		bool LoadImageFile(const std::filesystem::path& path);
		bool WriteToFile(const std::string& path);

		void Draw(Window& window);
	};
}

#endif