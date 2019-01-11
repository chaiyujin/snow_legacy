#pragma once
#include <memory>
#include <string>
#include <string.h>
#include "tool_log.h"
#include "memo_allocator.h"

namespace snow {

class SNOW_API Image {
private:
    using bytes = std::shared_ptr<uint8_t>;
    bytes mData;
    int   mWidth, mHeight, mBPP;
    int   mRealSize;
public:
    Image(): mData(nullptr, memory::deallocate)
           , mWidth(0), mHeight(0), mBPP(0), mRealSize(0) {}
    Image(int w, int h, int bpp) : Image() { this->resize(w, h, bpp); }
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

    /* static utils */
    
    // static Image Read(std::string filename);
    // static Image Save(std::string filename);
    // static void  FlipX(Image &image);
    // static void  FlipY(Image &image);
    // static Image MergeX(const Image &a, const Image &b);
    // static Image MergeY(const Image &a, const Image &b);
};

}