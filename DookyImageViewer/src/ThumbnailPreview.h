#ifndef THUMBNAILPREVIEW_H
#define THUMBNAILPREVIEW_H

#include <filesystem>
#include <unordered_map>

#include "Image.h"
#include "ImageUtils.h"
#include "Window.h"
#include "GUI.h"
#include "Text.h"

namespace Dooky {
	struct Thumbnail {
		Image image;
		std::filesystem::path filePath;
		int offset;
		int listIndex;
	};

	class ThumbnailPreview {
	private:
		std::vector<std::filesystem::path> browsingList;
		std::vector<Thumbnail*> previewImages;
		Thumbnail* centerPreviewImage;
		Image background;
		Image selectionBox;
		Image hoverBox;
		Text hoverText;

		bool isVisible;

		glm::ivec2 position;
		int width;
		int currentIndex;
		int centerOffset;

		int padding;
		int centerImagePadding;
		int thumbnailSize;

		int clickedIndex;
		int showHoverBox;
	public:
		ThumbnailPreview();
		~ThumbnailPreview();
		
		void SetVisible(bool visible);

		void SetPositionAndWidth(glm::ivec2 pos, int width);
		int GetClickedIndex();

		void ChangeBrowsingListAndIndex(const std::vector<std::filesystem::path>& newBrowsingList, int index);
		void ChangeIndex(int index);

		void HandleInteraction(Window& window, GUI& gui);
		void Draw(Window& window);
	};
}

#endif