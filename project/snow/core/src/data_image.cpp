#include "data_image.h"
#include <memory.h>
#include <algorithm>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../third-party/stb/stb_image.h"
#include "../third-party/stb/stb_image_write.h"

namespace snow {

void Image::resize(int w, int h, int bpp) {
    const int newSize = w*h*bpp;
    if (newSize > mRealSize) {
        mData.reset(); // release memory (if count==1)
        mData.reset(memory::allocate<uint8_t>(newSize),
                    memory::deallocate);
        mRealSize = newSize;
    }
    mWidth = w;
    mHeight = h;
    mBPP = bpp;
    this->zero();
}

}
