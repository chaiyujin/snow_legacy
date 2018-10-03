#include "snow_image.h"
#include "snow_path.h"
#include <memory.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

namespace snow {

Image Image::Read(std::string filename) {
    if (!path::exists(filename)) throw std::runtime_error("[Image]: read() file not found");
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
    else if (axis == 1) {
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
}

}