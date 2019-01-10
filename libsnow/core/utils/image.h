#pragma once
#include <memory>
#include <string>
#include <string.h>
#include "log.h"

namespace snow {

class Image {
private:
    using bytes = std::shared_ptr<uint8_t>;
    bytes mData;
    int   mWidth, mHeight, mBPP;
public:
    Image(): mData(nullptr), mWidth(0), mHeight(0), mBPP(0) {}
    Image(int w, int h, int bpp) : Image() { this->resize(w, h, bpp); }
    void resize(int w, int h, int bpp);
    // access to data
    uint8_t * data() { return mData.get(); }
    const uint8_t * data() const { return mData.get(); }
    // access to attributes
    constexpr int width()  const { return mWidth;  }
    constexpr int height() const { return mHeight; }
    constexpr int bpp()    const { return mBPP;    }
    constexpr int pixels() const { return mWidth*mHeight;      }
    constexpr int size()   const { return mWidth*mHeight*mBPP; }

    /* static utils */
    
    static Image Read(std::string filename);
    static Image Save(std::string filename);
    static void  FlipX(Image &image);
    static void  FlipY(Image &image);
    static Image MergeX(const Image &a, const Image &b);
    static Image MergeY(const Image &a, const Image &b);
};

}