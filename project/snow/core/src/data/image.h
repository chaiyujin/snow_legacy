#pragma once
#include <memory>
#include <string>
#include <string.h>
#include "../tools/log.h"
#include "../memory/allocator.h"

namespace snow {

class SNOW_API Image {
private:
    using bytes = std::shared_ptr<uint8_t>;
    bytes mData;
    int   mWidth, mHeight, mBPP;
    int   mRealSize;
public:
    /* static methods */
    
    static Image Load(std::string filename);
    static bool  Save(std::string filename, const Image &img, bool makeDir=false);
    static void  FlipX(Image &img);
    static void  FlipY(Image &img);
    static Image MergeX(const Image &a, const Image &b);
    static Image MergeY(const Image &a, const Image &b);

    /* constructor */
    Image(): mData(nullptr, memory::deallocate)
           , mWidth(0), mHeight(0), mBPP(0), mRealSize(0) {}
    Image(int w, int h, int bpp) : Image() { this->resize(w, h, bpp); }
    Image(const Image &b)
        : mData(b.mData)
        , mWidth(b.mWidth), mHeight(b.mHeight), mBPP(b.mBPP)
        , mRealSize(b.mRealSize) {}
    Image clone() const;
    void resize(int w, int h, int bpp);
    // clear
    void zero() { memset(mData.get(), 0, this->size()); }
    // access to data
    uint8_t * data() { return mData.get(); }
    const uint8_t * data() const { return mData.get(); }
    // access to attributes
    const int width()    const { return mWidth;  }
    const int height()   const { return mHeight; }
    const int bpp()      const { return mBPP;    }
    const int pixels()   const { return mWidth*mHeight;      }
    const int size()     const { return mWidth*mHeight*mBPP; }
    const int realSize() const { return mRealSize; }
    // access to static methods
    bool save(const std::string &filename, bool makeDir=false) { return Image::Save(filename, *this, makeDir); }
    Image &flipX() { Image::FlipX(*this); return *this; }
    Image &flipY() { Image::FlipY(*this); return *this; }
    Image flippedX() { Image ret(this->clone()); Image::FlipX(ret); return ret; }
    Image flippedY() { Image ret(this->clone()); Image::FlipY(ret); return ret; }
};

}