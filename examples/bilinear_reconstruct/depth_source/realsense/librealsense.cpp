#include "librealsense.h"


namespace librealsense {
#define STRCASE(T, X) case RS2_##T##_##X: {\
        static const std::string s##T##_##X##_str = make_less_screamy(#X);\
        return s##T##_##X##_str.c_str(); }

std::string make_less_screamy(const char* str)
{
    std::string res(str);

    bool first = true;
    for (auto i = 0; i < res.size(); i++)
    {
        if (res[i] != '_')
        {
            if (!first) res[i] = tolower(res[i]);
            first = false;
        }
        else
        {
            res[i] = ' ';
            first = true;
        }
    }
    return res;
}

const char* rs2_distortion_to_string(rs2_distortion value)
{
#define CASE(X) STRCASE(DISTORTION, X)
    switch (value)
    {
        CASE(NONE)
        CASE(MODIFIED_BROWN_CONRADY)
        CASE(INVERSE_BROWN_CONRADY)
        CASE(FTHETA)
        CASE(BROWN_CONRADY)
    default: return "UNKNOWN_VALUE";
    }
#undef CASE
}

const char* rs2_format_to_string(rs2_format value)
{
#define CASE(X) case RS2_FORMAT_##X: return #X;
    switch (value)
    {
        CASE(ANY)
        CASE(Z16)
        CASE(DISPARITY16)
        CASE(DISPARITY32)
        CASE(XYZ32F)
        CASE(YUYV)
        CASE(RGB8)
        CASE(BGR8)
        CASE(RGBA8)
        CASE(BGRA8)
        CASE(Y8)
        CASE(Y16)
        CASE(RAW10)
        CASE(RAW16)
        CASE(RAW8)
        CASE(UYVY)
        CASE(MOTION_RAW)
        CASE(MOTION_XYZ32F)
        CASE(GPIO_RAW)
        CASE(6DOF)
    default: return "UNKNOWN_VALUE";
    }
#undef CASE
}

#undef STRCASE
}