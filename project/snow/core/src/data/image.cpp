#include "image.h"
#include "../tools/path.h"
#include <memory.h>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../third-party/stb/stb_image.h"
#include "../../third-party/stb/stb_image_write.h"

namespace snow {

void  Image::_check(uint32_t w, uint32_t h, uint32_t bpp) const {
    if (w <= 0) log::fatal("[Image]: given w `{}` <= 0", w);
    if (h <= 0) log::fatal("[Image]: given h `{}` <= 0", h);
    if (bpp != 1 && bpp != 3 && bpp != 4)
        log::fatal("[Image]: only support bpp == (1, 3, 4). (Gray, RGB, RGBA)");
}

Image Image::clone() const {
    Image ret(mWidth, mHeight, mBPP);
    memcpy(ret.data(), this->data(), ret.size());
    return ret;
}

void Image::resize(uint32_t w, uint32_t h, uint32_t bpp) {
    this->_check(w, h, bpp);
    const uint32_t newSize = w*h*bpp;
    if (newSize > mRealSize) {
        // mData.reset(); // release memory (if count==1)
        mData.reset(memory::allocate<uint8_t>(newSize),
                    memory::deallocate);
        mRealSize = newSize;
    }
    mWidth = w;
    mHeight = h;
    mBPP = bpp;
    this->zero();
}

void Image::setData(const uint8_t *ptr, uint32_t w, uint32_t h, uint32_t bpp) {
    resize(w, h, bpp);
    memcpy(this->data(), ptr, this->size());
}

/* static methods */

Image Image::Load(std::string filename) {
    if (!path::exists(filename)) { 
        log::fatal("[Image]: read() file not found");
        return Image();
    } else {
        int w, h, n;
        uint8_t *data = stbi_load(filename.c_str(), &w, &h, &n, STBI_rgb_alpha);
        n = STBI_rgb_alpha;
        Image image(w, h, n);
        memcpy(image.data(), data, w * h * n);
        stbi_image_free(data);
        return image;
    }
}

bool  Image::Save(std::string filename, const Image &image, bool makeDir) {
    if (!path::exists(path::dirname(filename))) {
        log::debug("not exists {}", path::dirname(filename));
        if (!makeDir || !path::makedirs(filename, true)) {
            log::error("[Image]: save() failed to create file: {}", filename);
            return false;
        }
    }
    if (!(path::extension(filename) == ".png")) {
        log::warn("[Image]: only support .png");
        filename = path::change_extension(filename, ".png");
    }
    stbi_write_png(filename.c_str(), image.width(), image.height(), image.bpp(),
                   image.data(), image.width() * image.bpp());
    return true;
}

void  Image::FlipX(Image &image) {
    // flip cols
    uint8_t *data;
    for (uint32_t r = 0; r < image.mHeight; ++r) {
        data = image.data() + r * image.mWidth * image.mBPP;
        for (uint32_t c = 0; c < image.mWidth/2; ++c) {
            uint32_t i = c * image.mBPP;
            uint32_t j = (image.mWidth - 1 - c) * image.mBPP;
            for (uint32_t k = 0; k < image.mBPP; ++k) {
                std::swap(data[i + k], data[j + k]);
            }
        }
    }
}

void  Image::FlipY(Image &image) {
    // flip rows
    uint8_t *data0;
    uint8_t *data1;
    for (uint32_t r = 0; r < image.mHeight/2; ++r) {
        data0 = image.data() + r * image.mWidth * image.mBPP;
        data1 = image.data() + (image.mHeight - 1 - r) * image.mWidth * image.mBPP;
        for (uint32_t c = 0; c < image.mWidth; ++c) {
            uint32_t i = c * image.mBPP;
            for (uint32_t k = 0; k < image.mBPP; ++k) {
                std::swap(data0[i + k], data1[i + k]);
            }
        }
    }
}

Image Image::MergeX(const Image &image0, const Image &image1) {
    if (image0.size() == 0) return image1;
    if (image1.size() == 0) return image0;
    /* merge cols */ {
        log::assertion(image0.bpp() == image1.bpp(),
                       "[Image]: MergeX() two images have different bpp()");
        uint32_t newW = image0.width() + image1.width();
        uint32_t newH = std::max(image0.height(), image1.height());
        uint32_t newBPP = image0.bpp();
        Image image(newW, newH, newBPP);
        uint8_t *tar;
        const uint8_t *src;
        for (uint32_t r = 0; r < newH; ++r) {
            tar = image.data() + r * image.mWidth * newBPP;
            src = image0.data() + r * image0.mWidth * newBPP;
            for (uint32_t c = 0; c < image0.mWidth * newBPP; ++c) *tar++ = *src++;
            src = image1.data() + r * image1.mWidth * newBPP;
            for (uint32_t c = 0; c < image1.mWidth * newBPP; ++c) *tar++ = *src++;
        }
        return image;
    }
}

Image Image::MergeY(const Image &image0, const Image &image1) {
    if (image0.size() == 0) return image1;
    if (image1.size() == 0) return image0;
    /* merge rows */ {
        log::assertion(image0.bpp() == image1.bpp(),
                       "[Image]: MergeY() two images have different bpp()");
        uint32_t newW = std::max(image0.width(), image1.width());
        uint32_t newH = image0.height() + image1.height();
        uint32_t newBPP = image0.bpp();
        Image image(newW, newH, newBPP);
        uint8_t *tar;
        const uint8_t *src;
        for (uint32_t r = 0; r < newH; ++r) {
            tar = image.data() + r * image.mWidth * newBPP;
            if (r < image0.height()) src = image0.data() + r * image0.mWidth * newBPP;
            else                     src = image1.data() + (r - image0.mHeight) * image1.mWidth * newBPP;
            for (uint32_t c = 0; c < image0.mWidth * newBPP; ++c) *tar ++ = *src++;
        }
        return image;
    }
}

}
