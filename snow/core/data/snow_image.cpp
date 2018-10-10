#include "snow_image.h"
#include "../tools/snow_path.h"
#include <memory.h>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

namespace snow {

Image Image::Read(std::string filename) {
    if (!path::Exists(filename)) throw std::runtime_error("[Image]: read() file not found");
    int w, h, n;
    uint8_t *data = stbi_load(filename.c_str(), &w, &h, &n, STBI_rgb_alpha);
    n = STBI_rgb_alpha;
    Image image;
    image.resize(w, h, n);
    memcpy(image.data(), data, w * h * n);
    stbi_image_free(data);
    return image;
}

void Image::Write(std::string filename, const Image &image) {
    stbi_write_png(filename.c_str(), image.width(), image.height(), image.bpp(), image.data(), image.width() * image.bpp());
}

void Image::Flip(Image &image, int axis) {
    if (axis == 0) {
        // flip rows
        uint8_t *data0;
        uint8_t *data1;
        for (int r = 0; r < image.mHeight/2; ++r) {
            data0 = image.data() + r * image.mWidth * image.mBytesPerPixel;
            data1 = image.data() + (image.mHeight - 1 - r) * image.mWidth * image.mBytesPerPixel;
            for (int c = 0; c < image.mWidth; ++c) {
                int i = c * image.mBytesPerPixel;
                for (int k = 0; k < image.mBytesPerPixel; ++k) {
                    uint8_t tmp = data0[i + k];
                    data0[i + k] = data1[i + k];
                    data1[i + k] = tmp;
                }
            }
        }
    }
    else if (axis == 1) {
        // flip cols
        uint8_t *data;
        for (int r = 0; r < image.mHeight; ++r) {
            data = image.data() + r * image.mWidth * image.mBytesPerPixel;
            for (int c = 0; c < image.mWidth/2; ++c) {
                int i = c * image.mBytesPerPixel;
                int j = (image.mWidth - 1 - c) * image.mBytesPerPixel;
                for (int k = 0; k < image.mBytesPerPixel; ++k) {
                    uint8_t tmp = data[i + k];
                    data[i + k] = data[j + k];
                    data[j + k] = tmp;
                }
            }
        }
    }
    else throw std::runtime_error("[Image]: axis should be 0 (rows) or 1 (height)");
}


Image Image::Merge(const Image &image0, const Image &image1, int axis) {
    if (image0.size() == 0) return image1;
    if (image1.size() == 0) return image0;
    if (axis == 0) {
        // merge rows
        if (image0.bpp() != image1.bpp()) throw std::runtime_error("[Image]: Merge() two images have different bpp.");
        int newW = std::max(image0.width(), image1.width());
        int newH = image0.height() + image1.height();
        int newBPP = image0.bpp();
        Image image(newW, newH, newBPP);
        uint8_t *tar;
        const uint8_t *src;
        for (int r = 0; r < newH; ++r) {
            tar = image.data() + r * image.mWidth * newBPP;
            if (r < image0.height()) src = image0.data() + r * image0.mWidth * newBPP;
            else                     src = image1.data() + (r - image0.mHeight) * image1.mWidth * newBPP;
            for (int c = 0; c < image0.mWidth * newBPP; ++c) *tar ++ = *src++;
        }
        return image;
    }
    else if (axis == 1) {
        // merge cols
        if (image0.bpp() != image1.bpp()) throw std::runtime_error("[Image]: Merge() two images have different bpp.");
        int newW = image0.width() + image1.width();
        int newH = std::max(image0.height(), image1.height());
        int newBPP = image0.bpp();
        Image image(newW, newH, newBPP);
        uint8_t *tar;
        const uint8_t *src;
        for (int r = 0; r < newH; ++r) {
            tar = image.data() + r * image.mWidth * newBPP;
            src = image0.data() + r * image0.mWidth * newBPP;
            for (int c = 0; c < image0.mWidth * newBPP; ++c) *tar++ = *src++;
            src = image1.data() + r * image1.mWidth * newBPP;
            for (int c = 0; c < image1.mWidth * newBPP; ++c) *tar++ = *src++;
        }
        return image;
    }
    else throw std::runtime_error("[Image]: axis should be 0 (rows) or 1 (height)");
}

}