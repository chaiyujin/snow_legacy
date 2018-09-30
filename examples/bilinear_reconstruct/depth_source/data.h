#pragma once
#include <snow.h>
#include <vector>
#include <memory>

class Image {
    std::shared_ptr<uint8_t> mData;
    int                      mWidth, mHeight;
    int                      mPixels;
    int                      mBytesPerPixel;
public:
    Image(): mData(nullptr), mWidth(0), mHeight(0), mPixels(0), mBytesPerPixel(0) {}
    Image(int w, int h, int bpp) : Image() { alloc(w, h, bpp); }
    ~Image() {}
    void alloc(int w, int h, int bpp) {
        mWidth          = w;
        mHeight         = h;
        mPixels         = w * h;
        mBytesPerPixel  = bpp;
        mData.reset(new uint8_t[w * h * bpp]);
    }
    int             width()     const { return mWidth;          }
    int             height()    const { return mHeight;         }
    int             bpp()       const { return mBytesPerPixel;  }
    int             pixels()    const { return mPixels;         }
    uint8_t *       data()            { return mData.get();     }
    const uint8_t * data()      const { return mData.get();     }
    size_t          size()      const { return mPixels * mBytesPerPixel; }
    
    void reshape(int w, int h, int bpp) { 
        if (w * h * bpp != mPixels * mBytesPerPixel) throw std::runtime_error("[Image]: reshape() give different pixels.");
        mWidth = w;
        mHeight = h;
        mBytesPerPixel = bpp;
        mPixels = mWidth * mHeight;
    }

    static void ColorizeDepth(const Image &depth, Image &colorized) {
        if (depth.bpp() != 2) throw std::runtime_error("[Image]: colorizeDepth() input is not z16.");
        if (colorized.size() != depth.size() * 2)
            colorized.alloc(depth.width(), depth.height(), depth.bpp() * 2);
        colorized.reshape(depth.width(), depth.height(), depth.bpp() * 2);
        
        memset(colorized.data(), 0, colorized.size());
        const uint16_t *depthPtr = (uint16_t *)depth.data();
		int hist[65536];
		memset(hist, 0, sizeof(hist));
		for (int i = 0; i < depth.pixels(); ++i) {
			if (!depthPtr[i]) continue;
			++hist[depthPtr[i]];
		}
		for (int i = 1; i < 65536; ++i) hist[i] += hist[i - 1];

        auto cmap = snow::color_map::jet();
#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < depth.pixels(); ++i) {
			if (depthPtr[i] == 0) continue;
			float val = (float)hist[depthPtr[i]] / hist[0xFFFF];
			auto v = cmap.get(val);
			colorized.data()[i * colorized.bpp()]     = (uint8_t)v.x;
			colorized.data()[i * colorized.bpp() + 1] = (uint8_t)v.y;
			colorized.data()[i * colorized.bpp() + 2] = (uint8_t)v.z;
			colorized.data()[i * colorized.bpp() + 3] = 255;
		}
    }
};

/**
 * Point Cloud in RGB Camera space
 * */
class PointCloud {
	std::vector<snow::float3> mVert;
	std::vector<snow::float3> mNorm;
	std::vector<snow::float2> mTex;
	int					      mSize;
	int					      mWidth, mHeight;
public:
	PointCloud() : mSize(0), mWidth(0), mHeight(0) {}
	~PointCloud() {}
    /* set */
	void                                resize(int size)                { if (size != mSize) { mSize = size; mVert.resize(size); mNorm.resize(size); mTex.resize(size); } }
    void                                setWidth(int w)                 { mWidth = w;           }
    void                                setHeight(int h)                { mHeight = h;          }
    /* get */
	int                                 size()                    const { return mSize;         }
	int                                 width()                   const { return mWidth;        }
	int                                 height()                  const { return mHeight;       }
	snow::float3 &                      vertex(int i)                   { return mVert[i];      }
	snow::float3 &                      normal(int i)                   { return mNorm[i];      }
	snow::float2 &                      textureCoord(int i)             { return mTex[i];       }
	const snow::float3 &                vertex(int i)             const { return mVert[i];      }
	const snow::float3 &                normal(int i)             const { return mNorm[i];      }
	const snow::float2 &                textureCoord(int i)       const { return mTex[i];       }
	std::vector<snow::float3> &         verticeList()                   { return mVert;         }
	std::vector<snow::float3> &         normalList()                    { return mNorm;         }
	std::vector<snow::float2> &         textureCoordList ()             { return mTex;          }
	const std::vector<snow::float3> &   verticeList()             const { return mVert;         }
	const std::vector<snow::float3> &   normalList()              const { return mNorm;         }
	const std::vector<snow::float2> &   textureCoordList()        const { return mTex;          }
};

class MorphModel {
    std::vector<snow::float3>       mVert;
    std::vector<snow::float3>       mNorm;
    std::vector<snow::float2>       mTex;
    std::vector<snow::int3>         mIndices;
	int					            mNumVertice;
public:
    MorphModel(): mVert(0), mNorm(0), mTex(0), mIndices(0), mNumVertice(0) {}
    ~MorphModel() {}
    /* set */
    void                                resize(int points)              { if (points != mNumVertice) { mNumVertice = points; mVert.resize(points); mNorm.resize(points); mTex.resize(points); } }
    void                                setIndices(const std::vector<snow::int3> &indices) { mIndices = indices; }
    /* get */
    int                                 numVertices()             const { return mNumVertice;   }
	snow::float3 &                      vertex(int i)                   { return mVert[i];      }
	snow::float3 &                      normal(int i)                   { return mNorm[i];      }
	snow::float2 &                      textureCoord(int i)             { return mTex[i];       }
	const snow::float3 &                vertex(int i)             const { return mVert[i];      }
	const snow::float3 &                normal(int i)             const { return mNorm[i];      }
	const snow::float2 &                textureCoord(int i)       const { return mTex[i];       }
	std::vector<snow::float3> &         verticeList()                   { return mVert;         }
	std::vector<snow::float3> &         normalList()                    { return mNorm;         }
	std::vector<snow::float2> &         textureCoordList ()             { return mTex;          }
	const std::vector<snow::float3> &   verticeList()             const { return mVert;         }
	const std::vector<snow::float3> &   normalList()              const { return mNorm;         }
	const std::vector<snow::float2> &   textureCoordList()        const { return mTex;          }
    const std::vector<snow::int3> &     indices()                 const { return mIndices;      }
};
