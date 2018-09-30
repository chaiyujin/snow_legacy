#include "rsutils.h"
#include <fstream>

namespace librealsense {

RealSenseSource::RealSenseSource() {}
RealSenseSource::RealSenseSource(std::string paramPath) : RealSenseSource() { this->initFrom(paramPath); }
RealSenseSource::~RealSenseSource() {}

void RealSenseSource::initFrom(std::string paramPath) {
    std::ifstream fin(paramPath);
    if (!fin.is_open()) throw std::runtime_error("Failed to open parameter path.");
    // begin to read
    std::string name;
    // read intrinsics and extrinsics
    for (int i = 0; i < 2; ++i) {
        std::getline(fin, name);
        name = name.substr(0, 5);
        if      (name == "Depth") fin >> mDepthIntrinsics;
        else if (name == "Color") fin >> mColorIntrinsics;
    }
    for (int i = 0; i < 2; ++i) {
        std::getline(fin, name);
        name = name.substr(0, 14);
        if      (name == "Depth to Color") fin >> mDepth2ColorExtrinsics;
        else if (name == "Color to Depth") fin >> mColor2DepthExtrinsics;
    }
    // alloc image
    mColorImg.alloc(         mColorIntrinsics.width, mColorIntrinsics.height, COLOR_BPP);
    mDepthImg.alloc(         mDepthIntrinsics.width, mDepthIntrinsics.height, DEPTH_BPP);
    mAlignedColorImg.alloc(  mDepthIntrinsics.width, mDepthIntrinsics.height, COLOR_BPP);
    mColorizedDepthImg.alloc(mDepthIntrinsics.width, mDepthIntrinsics.height, COLOR_BPP);
    // depth scale
    while (name.substr(0, 5) != "Depth") { std::getline(fin, name); }
    while (true) {
        int id; float val;
        fin >> id;
        if (id == -1) break;
        fin >> val;
        std::getline(fin, name);
        snow::Trim(name);
        if (name == "Depth Units") {
            // std::cout << id << " " << val << " " << name << std::endl;
            mDepthScale = val;
            break;
        }
    }
    // close file
    fin.close();
#ifdef TEST_REALSENSE
    std::cout
        << "==========================\n"
        << "-----   Color Intr   -----\n" << mColorIntrinsics
        << "-----   Depth Intr   -----\n" << mDepthIntrinsics
        << "----- Depth to Color -----\n" << mDepth2ColorExtrinsics
        << "----- Color to Depth -----\n" << mColor2DepthExtrinsics
        << "-----   DepthScale   ----- " << mDepthScale << "\n"
        << "==========================\n";
#endif
}

void RealSenseSource::updateFramePair(const uint8_t *color, const uint8_t *depth) {
    memcpy(mColorImg.data(), color, mColorImg.pixels() * mColorImg.bpp());
    memcpy(mDepthImg.data(), depth, mDepthImg.pixels() * mDepthImg.bpp());
}

void RealSenseSource::updatePointCloud() {
    const uint16_t *depthPtr = (const uint16_t *)mDepthImg.data();
    // set point cloud members
    mPointCloud.setWidth(mDepthIntrinsics.width);
    mPointCloud.setHeight(mDepthIntrinsics.height);
    mPointCloud.resize(mDepthImg.pixels());
    // iter
    for (int iPixel = 0; iPixel < mDepthImg.pixels(); ++iPixel) {
        if (depthPtr[iPixel] == 0) {
            float val = 0.f; // std::numeric_limits<float>::infinity()
            mPointCloud.verticeList()[iPixel].x = val;
            mPointCloud.verticeList()[iPixel].y = val;
            mPointCloud.verticeList()[iPixel].z = val;
        }
        else {
            int x = iPixel % mDepthIntrinsics.width;
            int y = iPixel / mDepthIntrinsics.width;
            float pixel[2] = {(float)x, (float)y};
            snow::float3 colorSpace;
            snow::float2 pix;
            rs2_deproject_pixel_to_point(&mPointCloud.verticeList()[iPixel], &mDepthIntrinsics, pixel, (float)depthPtr[iPixel] * mDepthScale);
            rs2_transform_point_to_point(&colorSpace, &mDepth2ColorExtrinsics, &mPointCloud.verticeList()[iPixel]);
            rs2_project_point_to_pixel(&pix, &mColorIntrinsics, &colorSpace);
            // set texture
            mPointCloud.textureCoordList()[iPixel].x = pix.x / (float)mColorIntrinsics.width;
            mPointCloud.textureCoordList()[iPixel].y = pix.y / (float)mColorIntrinsics.height;
            // set vertice in colorSpace
            mPointCloud.verticeList()[iPixel].x = colorSpace.x;
            mPointCloud.verticeList()[iPixel].y = colorSpace.y;
            mPointCloud.verticeList()[iPixel].z = colorSpace.z;
        }
    }
}

glm::mat4 RealSenseSource::projectMat(const rs2_intrinsics &intr) const {
    float f = 100.f, n = 0.1f;
    float a = -(f + n) / (f - n);
    float b = -2.f * f * n / (f - n);
    glm::mat4 proj(0.0f);
    proj[0][0] = intr.fx * 2.0f / intr.width;            proj[0][1] = 0;                                  proj[0][2] = 0; proj[0][3] = 0;
    proj[1][0] = 0;                                      proj[1][1] = intr.fy * 2.f  / intr.height;       proj[1][2] = 0; proj[1][3] = 0;
    proj[2][0] = -(intr.ppx * 2.0f / intr.width - 1.f);  proj[2][1] = intr.ppy * 2.f / intr.height - 1.f; proj[2][2] = a; proj[2][3] = -1.f;
    proj[3][0] = 0;                                      proj[3][1] = 0;                                  proj[3][2] = b; proj[3][3] = 0;  
    return proj;
}

glm::mat4 RealSenseSource::transformMat(const rs2_extrinsics &extr) const {
    glm::mat4 R(1.0);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            R[i][j] = extr.rotation[i * 3 + j];
        }
    }
    glm::mat4 T(1.0);
    for (int i = 0; i < 3; ++i)
        T[3][i] = extr.translation[i];
    return T * R;
}
}