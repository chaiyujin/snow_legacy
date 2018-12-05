#pragma once
#include <string.h>
#include "ubuntu_l.h"
#include "arial.h"
namespace snow {

const char *SupportingFonts[] = { "ubuntu", "arial" };

inline bool getFont(const char *fontName, const unsigned char **bufferPtr, int *sizePtr) {
    if (strcmp(fontName, "ubuntu") == 0) {
        *bufferPtr = Ubuntu_L_ttf;
        *sizePtr = Ubuntu_L_ttf_len;
        return true;
    }
    else if (strcmp(fontName, "arial") == 0) {
        *bufferPtr = arial_ttf;
        *sizePtr = arial_ttf_len;
        return true;
    }
    else
        return false;
}

}