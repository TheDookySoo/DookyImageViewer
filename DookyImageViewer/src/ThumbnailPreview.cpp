#include "ThumbnailPreview.h"
#include "ImageUtils.h"

#include "StringUtils.h"


namespace Dooky {
	ThumbnailPreview::ThumbnailPreview() {
		isVisible = true;

		position = { 0, 0 };
		width = 400;
		currentIndex = 0;
		centerOffset = 0;

		padding = 4;
		centerImagePadding = 0;
		thumbnailSize = 64;

		clickedIndex = -1;
		showHoverBox = false;

		background.Create(1, 1, { 0.4f, 0.4f, 0.4f, 1.0f });

		hoverBox.Create(1, 1, { 0.2f, 0.5f, 1.0f, 0.5f });
		hoverBox.adjustment_ShowAlphaCheckerboard = false;

		hoverText.LoadFontFromPath("./resources/fonts/Consolas.ttf", 12);
		hoverText.SetColor(1.0f, 1.0f, 1.0f);
		hoverText.SetString("Text.");
		hoverText.SetAnchorPoint(0.5f, 0.0f);
	}

	ThumbnailPreview::~ThumbnailPreview() {
		
	}

	////////////////////////////////////////
	///// MISC
	////////////////////////////////////////

	Thumbnail* CreateThumbnail(std::filesystem::path path) {
		FileThumbnailImage thumb = GetImageFileThumbnail(path, 64);
		Thumbnail* thumbnail = new Thumbnail;
		thumbnail->offset = 0;
		thumbnail->filePath = path;

		if (thumb.success) {
			thumbnail->image.LoadRawData(thumb.width, thumb.height, thumb.bitmap);
		} else {
			if (!thumbnail->image.LoadImageFile("./resources/images/NoImage.png")) {
				thumbnail->image.Create(64, 64, { 1.0f, 0.0f, 1.0f, 1.0f });
			}

			thumbnail->image.FlipVertically(true);
		}

		thumbnail->image.adjustment_ShowAlphaCheckerboard = false;

		return thumbnail;
	};

	////////////////////////////////////////
	///// PRIVATE
	////////////////////////////////////////

	////////////////////////////////////////
	///// PUBLIC
	////////////////////////////////////////

	void ThumbnailPreview::SetVisible(bool visible) {
		isVisible = visible;
	}

	void ThumbnailPreview::SetPositionAndWidth(glm::ivec2 pos, int width) {
		position = pos;
		this->width = width;
	}

	int ThumbnailPreview::GetClickedIndex() {
		int copy = clickedIndex;
		clickedIndex = -1;

		return copy;
	}

	void ThumbnailPreview::ChangeBrowsingListAndIndex(const std::vector<std::filesystem::path>& newBrowsingList, int index) {
		if (newBrowsingList.empty())
			return;

		browsingList = newBrowsingList;

		// Delete
		
		for (Thumbnail* thumb : previewImages) {
			delete thumb;
		}

		previewImages.clear();

		// Create
		Thumbnail* thumb = CreateThumbnail(browsingList[index]);
		thumb->listIndex = index;
		thumb->image.SetAnchorPoint(0.5f, 0.5f);
		previewImages.push_back(thumb);
		centerPreviewImage = thumb;

		glm::ivec2 originalThumbSize = thumb->image.GetSize();

		int i = index;
		int originalOffset = thumb->image.GetSize().x / 2 + padding + centerImagePadding;

		// Create images
		for (int side = 0; side < 2; side++) {
			int increment = side == 0 ? -1 : 1;
			int offset = originalOffset;
			i = index;

			while (true) {
				i = i + increment;

				if (i >= 0 && i < browsingList.size()) {
					auto path = browsingList[i];
					thumb = CreateThumbnail(path);
					thumb->listIndex = i;
					previewImages.push_back(thumb);
					
					if (side == 0) {
						thumb->image.SetAnchorPoint(1.0f, 0.5f);
						thumb->offset = -offset;
					} else {
						thumb->image.SetAnchorPoint(0.0f, 0.5f);
						thumb->offset = offset;
					}

					offset += thumb->image.GetSize().x + padding;

					if (offset > width / 2) break;
				} else {
					break;
				}
			}
		}

		// Selection box
		selectionBox.SetAnchorPoint(0.5f, 0.5f);
		selectionBox.Create(originalThumbSize.x + 4, originalThumbSize.y + 4, { 0.2f, 0.8f, 1.0f });
		selectionBox.adjustment_ShowAlphaCheckerboard = false;

		for (int x = 2; x < originalThumbSize.x + 2; x++) {
			for (int y = 2; y < originalThumbSize.y + 2; y++) {
				selectionBox.SetPixel(x, y, { 0.0f, 0.0f, 0.0f, 0.0f });
			}
		}

		currentIndex = index;
	}

	void ThumbnailPreview::ChangeIndex(int index) {
		if (browsingList.empty())
			return;

		std::unordered_map<int, Thumbnail*> existingThumbnails;

		for (Thumbnail* thumb : previewImages) {
			existingThumbnails.insert(std::pair<int, Thumbnail*>(thumb->listIndex, thumb));
		}

		Thumbnail* thumb;

		auto foundThumb = existingThumbnails.find(index);
		if (foundThumb != existingThumbnails.end()) {
			thumb = foundThumb->second;
		} else {
			thumb = CreateThumbnail(browsingList[index]);
			thumb->listIndex = index;
		}

		int originalOffset = thumb->image.GetSize().x / 2 + padding + centerImagePadding;

		// Create images
		std::vector<Thumbnail*> newPreviewImages;

		glm::ivec2 originalThumbSize = thumb->image.GetSize();
		thumb->offset = 0;
		thumb->image.SetAnchorPoint(0.5f, 0.5f);
		newPreviewImages.push_back(thumb);

		int i = index;

		for (int side = 0; side < 2; side++) {
			int increment = side == 0 ? -1 : 1;
			int offset = originalOffset;
			i = index;

			while (true) {
				i = i + increment;

				if (i >= 0 && i < browsingList.size()) {
					auto path = browsingList[i];

					auto foundThumb = existingThumbnails.find(i);
					if (foundThumb != existingThumbnails.end()) {
						thumb = foundThumb->second;
					} else {
						thumb = CreateThumbnail(path);
						thumb->listIndex = i;
					}

					newPreviewImages.push_back(thumb);
					
					if (side == 0) {
						thumb->image.SetAnchorPoint(1.0f, 0.5f);
						thumb->offset = -offset;
					} else {
						thumb->image.SetAnchorPoint(0.0f, 0.5f);
						thumb->offset = offset;
					}

					offset += thumb->image.GetSize().x + padding;

					if (offset > width / 2) break;
				} else {
					break;
				}
			}
		}

		// Selection box
		selectionBox.SetAnchorPoint(0.5f, 0.5f);
		selectionBox.Create(originalThumbSize.x + 4, originalThumbSize.y + 4, { 0.2f, 0.8f, 1.0f });
		selectionBox.adjustment_ShowAlphaCheckerboard = false;

		for (int x = 2; x < originalThumbSize.x + 2; x++) {
			for (int y = 2; y < originalThumbSize.y + 2; y++) {
				selectionBox.SetPixel(x, y, { 0.0f, 0.0f, 0.0f, 0.0f });
			}
		}

		// Delete unused thumbnails
		for (Thumbnail* t1 : previewImages) {
			bool found = false;

			for (Thumbnail* t2 : newPreviewImages) {
				if (t1 == t2) {
					found = true;
					break;
				}
			}

			if (found == false) {
				//std::cout << "Deleted: " << t1->listIndex << std::endl;
				delete t1;
			}
		}

		previewImages = newPreviewImages;
		currentIndex = index;
	}

	void ThumbnailPreview::HandleInteraction(Window& window, GUI& gui) {
		if (!isVisible)
			return;

		showHoverBox = false;

		if (gui.imguiCaptureMouse)
			return;

		glm::ivec2 mousePos = window.GetMousePosition();
		glm::ivec2 backgroundPos = background.GetPosition();
		glm::vec2 backgroundScale = background.GetScale();
		bool clicked = window.WasMousePressed(GLFW_MOUSE_BUTTON_1);

		if (mousePos.x < backgroundPos.x || mousePos.x > backgroundPos.x + backgroundScale.x || mousePos.y < backgroundPos.y || mousePos.y > backgroundPos.y + backgroundScale.y)
			return;

		for (int i = 0; i < previewImages.size(); i++) {
			Thumbnail* thumb = previewImages[i];

			glm::ivec2 pos = thumb->image.GetPosition();
			glm::ivec2 size = thumb->image.GetSize();
			glm::ivec2 halfSize = size / 2;
			glm::vec2 anchor = thumb->image.GetAnchorPoint();

			int lowerBoundX = (pos.x - (size.x * (anchor.x - 0.5f))) - halfSize.x;
			int upperBoundX = (pos.x - (size.x * (anchor.x - 0.5f))) + halfSize.x;

			int lowerBoundY = pos.y - (size.y / 2);
			int upperBoundY = pos.y + (size.y / 2);

			if (mousePos.x >= lowerBoundX && mousePos.x <= upperBoundX && mousePos.y >= lowerBoundY && mousePos.y <= upperBoundY) {
				if (clicked) {
					clickedIndex = thumb->listIndex;
				}

				showHoverBox = true;
				hoverBox.SetScale(size.x, size.y);
				hoverBox.SetPosition(pos.x, pos.y);
				hoverBox.SetAnchorPoint(anchor.x, anchor.y);

				// Hover text
				std::string extension = "Unknown extension";

				try {
					extension = thumb->filePath.extension().string();
					LowerString(extension);
				} catch (std::exception& exception) {};

				hoverText.SetString(extension);

				break;
			}
		}
	}

	void ThumbnailPreview::Draw(Window& window) {
		if (!isVisible)
			return;

		int heightOffset = thumbnailSize / 2 + padding + position.y;

		background.SetPosition(0, position.y);
		background.SetScale(width, thumbnailSize + padding * 2);
		background.Draw(window);

		selectionBox.SetPosition(width / 2, heightOffset);

		// Draw thumbnails
		for (int i = 0; i < previewImages.size(); i++) {
			Thumbnail* thumb = previewImages[i];
			thumb->image.SetPosition(width / 2 + thumb->offset, heightOffset);
			thumb->image.Draw(window);
		}

		selectionBox.Draw(window);

		if (showHoverBox) {
			hoverText.SetPosition(
				(int)(hoverBox.GetPosition().x - (hoverBox.GetScale().x * (hoverBox.GetAnchorPoint().x - 0.5f))),
				(int)(hoverBox.GetPosition().y + (hoverBox.GetScale().y / 2))
			);

			hoverBox.Draw(window);
			hoverText.Draw(window);
		}
	}
}