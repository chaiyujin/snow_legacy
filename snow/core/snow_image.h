#pragma once
#include <memory>
#include <string>
#include <string.h>
namespace snow {

class Image {
private:
    std::shared_ptr<uint8_t> mData;
    int                      mWidth, mHeight;
    int                      mBytesPerPixel;
public:
    Image(int w=0, int h=0, int bpp=0) : mData(nullptr) { resize(w, h, bpp); }

    void resize(int w, int h, int bpp) {
        if (w * h * bpp > 0 && mWidth * mHeight * mBytesPerPixel != w * h * bpp) {
            mData.reset(new uint8_t[w * h* bpp]);
        }
        mWidth          = w;
        mHeight         = h;
        mBytesPerPixel  = bpp;
        zero();
    }

    uint8_t *       data()            { return mData.get();     }
    const uint8_t * data()      const { return mData.get();     }
    const int       width()     const { return mWidth;          }
    const int       height()    const { return mHeight;         }
    const int       bpp()       const { return mBytesPerPixel;  }
    int             size()      const { return mWidth * mHeight * mBytesPerPixel;                  }
    void            zero()            { memset(mData.get(), 0, mWidth * mHeight * mBytesPerPixel); }

    static Image Read(std::string filename);
    static void  Write(std::string filename, const Image &image);
    static void  Flip(Image &image, int axis);
    static Image Merge(const Image &image0, const Image &image1, int axis);
};


} // snow
