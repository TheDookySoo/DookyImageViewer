#include "ImageUtils.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <Windows.h>
#include <thumbcache.h>

namespace Dooky {
	FileThumbnailImage GetImageFileThumbnail(const std::filesystem::path& path, int targetSize) {
		std::wstring convertedWideString = path.wstring();
		std::replace(convertedWideString.begin(), convertedWideString.end(), '/', '\\');

		FileThumbnailImage thumbnailImage;
		thumbnailImage.width = 0;
		thumbnailImage.height = 0;
		
		try {
			HRESULT hr = CoInitialize(nullptr);

			if (FAILED(hr)) throw std::runtime_error("Failed CoInitialize in LoadImageFile");

			IShellItem* item = nullptr;
			hr = SHCreateItemFromParsingName(convertedWideString.c_str(), 0, IID_PPV_ARGS(&item));

			if (FAILED(hr)) throw std::runtime_error("Failed SHCreateItemFromParsingName in LoadImageFile");

			IThumbnailCache* cache = nullptr;
			hr = CoCreateInstance(CLSID_LocalThumbnailCache, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&cache));

			if (FAILED(hr)) throw std::runtime_error("Failed CoCreateInstance in LoadImageFile");

			ISharedBitmap* shared_bitmap;
			hr = cache->GetThumbnail(item, targetSize, WTS_EXTRACT | WTS_SCALETOREQUESTEDSIZE, &shared_bitmap, nullptr, nullptr);

			if (FAILED(hr)) throw std::runtime_error("Failed GetThumbnail in LoadImageFile");

			HBITMAP hBitmap = NULL;
			hr = shared_bitmap->GetSharedBitmap(&hBitmap);

			if (FAILED(hr)) throw std::runtime_error("Failed GetSharedBitmap in LoadImageFile");

			HDC dc = GetDC(NULL);
			HDC dc_mem = CreateCompatibleDC(dc);

			BITMAPINFO bmi;
			ZeroMemory(&bmi, sizeof(BITMAPINFO));
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			GetDIBits(dc_mem, hBitmap, 0, 0, nullptr, &bmi, DIB_RGB_COLORS);

			WTS_ALPHATYPE alpha_type;
			hr = shared_bitmap->GetFormat(&alpha_type);

			if (FAILED(hr)) throw std::runtime_error("Failed GetFormat in LoadImageFile");

			bmi.bmiHeader.biBitCount = alpha_type == WTSAT_RGB ? 24 : 32;
			bmi.bmiHeader.biHeight = std::abs(bmi.bmiHeader.biHeight);
			bmi.bmiHeader.biCompression = BI_RGB;

			// Get Image Data

			unsigned char* buffer = new unsigned char[bmi.bmiHeader.biSizeImage];
			GetDIBits(dc_mem, hBitmap, 0, bmi.bmiHeader.biHeight, buffer, &bmi, DIB_RGB_COLORS);

			int width = bmi.bmiHeader.biWidth;
			int height = bmi.bmiHeader.biHeight;
			int bitCount = bmi.bmiHeader.biBitCount;
			int bytesPerPixel = bitCount / 8;
			int stride = ((width * bytesPerPixel + 3) & ~3) % width;

			thumbnailImage.bitmap.resize(width * height * 4, 0);

			size_t bitmapIndex = 0;

			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					size_t index = y * width + x;

					if (bytesPerPixel == 3) {
						thumbnailImage.bitmap[index * 4 + 0] = (float)buffer[bitmapIndex + 2];
						thumbnailImage.bitmap[index * 4 + 1] = (float)buffer[bitmapIndex + 1];
						thumbnailImage.bitmap[index * 4 + 2] = (float)buffer[bitmapIndex + 0];
						thumbnailImage.bitmap[index * 4 + 3] = 255;
					} else if (bytesPerPixel == 4) {
						thumbnailImage.bitmap[index * 4 + 0] = (float)buffer[bitmapIndex + 2];
						thumbnailImage.bitmap[index * 4 + 1] = (float)buffer[bitmapIndex + 1];
						thumbnailImage.bitmap[index * 4 + 2] = (float)buffer[bitmapIndex + 0];
						thumbnailImage.bitmap[index * 4 + 3] = (float)buffer[bitmapIndex + 3];
					}

					bitmapIndex += bytesPerPixel;
				}

				bitmapIndex += stride;
			}

			thumbnailImage.width = width;
			thumbnailImage.height = height;
			thumbnailImage.success = true;

			item->Release();
			cache->Release();
			shared_bitmap->Detach(&hBitmap);
			shared_bitmap->Release();

			DeleteObject(item);
			DeleteObject(cache);
			DeleteObject(shared_bitmap);
			DeleteObject(hBitmap);
			ReleaseDC(NULL, dc);
			ReleaseDC(NULL, dc_mem);

			delete[] buffer;
		} catch (std::exception& exception) {
			//std::cout << "EXCEPTION: " << exception.what() << std::endl;
			thumbnailImage.success = false;
		}

		return thumbnailImage;
	}
}