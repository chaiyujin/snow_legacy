#pragma once
#include <string.h>
#include "arial.h"
#include "ubuntu_l.h"
#include "ubuntu_mono.h"
namespace snow {

const char *SupportingFonts[] = { "arial", "ubuntu", "ubuntu-mono" };

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
    else if (strcmp(fontName, "ubuntu-mono") == 0) {
        *bufferPtr = Ubuntu_Mono_Bold_ttf;
        *sizePtr = Ubuntu_Mono_Bold_ttf_len;
        return true;
    }
    else
        return false;
}

}