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
    bytes    mData;
    uint32_t mWidth, mHeight, mBPP;
    uint32_t mRealSize;

    void  _check(uint32_t w, uint32_t h, uint32_t bpp) const;
public:
    /* static methods */
    
    static Image Load(const std::string &filename);
    static bool  Save(const std::string &filename, const Image &img, bool makeDirs=false);
    static void  FlipX(Image &img);
    static void  FlipY(Image &img);
    static Image MergeX(const Image &a, const Image &b);
    static Image MergeY(const Image &a, const Image &b);

    /* constructor */
    Image()
        : mData(nullptr, memory::deallocate)
        , mWidth(0), mHeight(0), mBPP(0), mRealSize(0) {}
    Image(uint32_t w, uint32_t h, uint32_t bpp)
        : Image() { this->resize(w, h, bpp); }
    Image(const Image &b)
        : mData(b.mData)
        , mWidth(b.mWidth), mHeight(b.mHeight), mBPP(b.mBPP)
        , mRealSize(b.mRealSize) {}
    Image clone() const;
    bool isNull() const { return this->size() == 0; }
    void resize(uint32_t w, uint32_t h, uint32_t bpp);
    // clear
    void zero() { memset(mData.get(), 0, this->size()); }
    // access to data
    uint8_t * data() { return mData.get(); }
    const uint8_t * data() const { return mData.get(); }
    void setData(const uint8_t *ptr, uint32_t w, uint32_t h, uint32_t bpp);
    // access to attributes
    uint32_t width()    const { return mWidth;  }
    uint32_t height()   const { return mHeight; }
    uint32_t bpp()      const { return mBPP;    }
    uint32_t pixels()   const { return mWidth*mHeight;      }
    uint32_t size()     const { return mWidth*mHeight*mBPP; }
    uint32_t realSize() const { return mRealSize; }
    // access to static methods
    bool load(const std::string &filename);
    bool save(const std::string &filename, bool makeDirs=false) const;
    Image &flipX() { Image::FlipX(*this); return *this; }
    Image &flipY() { Image::FlipY(*this); return *this; }
    Image flippedX() { Image ret(this->clone()); Image::FlipX(ret); return ret; }
    Image flippedY() { Image ret(this->clone()); Image::FlipY(ret); return ret; }
};

}