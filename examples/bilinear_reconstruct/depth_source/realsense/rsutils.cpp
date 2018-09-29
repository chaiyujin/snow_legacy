#include "rsutils.h"
#include <fstream>

namespace librealsense_ext {

RealSenseSoftwareDevice::RealSenseSoftwareDevice()
    : mDevice()
    , mColorSensor(mDevice.add_sensor("Color"))
    , mDepthSensor(mDevice.add_sensor("Depth"))
    , mCount(0) {}
RealSenseSoftwareDevice::RealSenseSoftwareDevice(std::string paramPath)
    : RealSenseSoftwareDevice() {
    this->initFrom(paramPath);
}
RealSenseSoftwareDevice::~RealSenseSoftwareDevice() {
    mColorSensor.stop();
    mDepthSensor.stop();
}

void RealSenseSoftwareDevice::initFrom(std::string paramPath) {
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
    // add streams
    mColorProfile = mColorSensor.add_video_stream({
        RS2_STREAM_COLOR, 0, 0,
        mColorIntrinsics.width,
        mColorIntrinsics.height,
        FPS, COLOR_BPP,
        RS2_FORMAT_RGBA8,
        mColorIntrinsics
    });
    mDepthProfile = mDepthSensor.add_video_stream({
        RS2_STREAM_DEPTH, 0, 1,
        mDepthIntrinsics.width,
        mDepthIntrinsics.height,
        FPS, DEPTH_BPP,
        RS2_FORMAT_Z16,
        mDepthIntrinsics
    });
    mDepthProfile.register_extrinsics_to(mColorProfile, mDepth2ColorExtrinsics);
    // read sensor parameters
    for (int i = 0; i < 2; ++i) {
        std::getline(fin, name);
        name = name.substr(0, 5);
        if      (name == "Depth") fin >> mDepthSensor;
        else if (name == "Color") fin >> mColorSensor;
    }
    mDepthScale = mDepthSensor.get_option(RS2_OPTION_DEPTH_UNITS);
    // open
    mDevice.create_matcher(RS2_MATCHER_COUNT);
    mColorSensor.open(mColorProfile);
    mDepthSensor.open(mDepthProfile);
    mColorSensor.start(mSyncer);
    mDepthSensor.start(mSyncer);
    // close file
    fin.close();
#ifdef TEST_REALSENSE    
    std::cout
        << "==========================\n"
        << "-----   Color Intr   -----\n" << mColorIntrinsics
        << "-----   Depth Intr   -----\n" << mDepthIntrinsics
        << "----- Depth to Color -----\n" << mDepth2ColorExtrinsics
        << "----- Color to Depth -----\n" << mColor2DepthExtrinsics
        << "-----  Color Sensor  -----\n" << mColorSensor
        << "-----  Depth Sensor  -----\n" << mDepthSensor
        << "==========================\n";
#endif
}

void RealSenseSoftwareDevice::updateFramePair(const uint8_t *color, const uint8_t *depth) {
    memcpy(mColorImg.data(), color, mColorImg.pixels() * mColorImg.bpp());
    memcpy(mDepthImg.data(), depth, mDepthImg.pixels() * mDepthImg.bpp());
}

void RealSenseSoftwareDevice::updatePointCloud() {
    const uint16_t *depthPtr = (const uint16_t *)mDepthImg.data();
    // set point cloud members
    mPointCloud.setWidth(mDepthIntrinsics.width);
    mPointCloud.setHeight(mDepthIntrinsics.height);
    mPointCloud.resize(mDepthImg.pixels());
    // iter
    for (int iPixel = 0; iPixel < mDepthImg.pixels(); ++iPixel) {
        if (depthPtr[iPixel] == 0) {
            mPointCloud.verticeList()[iPixel].x = std::numeric_limits<float>::infinity();
            mPointCloud.verticeList()[iPixel].y = std::numeric_limits<float>::infinity();
            mPointCloud.verticeList()[iPixel].z = std::numeric_limits<float>::infinity();
        }
        else {
            int x = iPixel % mDepthIntrinsics.width;
            int y = iPixel / mDepthIntrinsics.width;
            float pixel[2] = {(float)x, (float)y};
            snow::float3 tmp;
            snow::float2 pix;
            rs2_deproject_pixel_to_point(&mPointCloud.verticeList()[iPixel], &mDepthIntrinsics, pixel, (float)depthPtr[iPixel] * mDepthScale);
            rs2_transform_point_to_point(&tmp, &mDepth2ColorExtrinsics, &mPointCloud.verticeList()[iPixel]);
            rs2_project_point_to_pixel(&pix, &mColorIntrinsics, &tmp);
            mPointCloud.textureCoordsList()[iPixel].x = pix.x / (float)mColorIntrinsics.width;
            mPointCloud.textureCoordsList()[iPixel].y = pix.y / (float)mColorIntrinsics.height;
            // _point_cloud._vert[k].x *= 1;
            // _point_cloud._vert[k].y *= 1;
            // _point_cloud._vert[k].z *= 1;
        }
    }
}

glm::mat4 RealSenseSoftwareDevice::projectMat(const rs2_intrinsics &intr) const {
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

}