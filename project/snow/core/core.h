#pragma once
#include "src/common.h"
#include "src/tools/log.h"
#include "src/tools/path.h"
#include "src/tools/timer.h"
#include "src/memory/allocator.h"
#include "src/data/image.h"
#include "src/data/wav.h"

namespace snow {

SNOW_INLINE std::string __version__(void) { return SNOW_VERSION_STRING; }

}