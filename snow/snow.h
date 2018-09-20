#pragma once

#include "core/snow_core.h"

// #ifdef SNOW_MODULE_OPENGL
#include "model/snow_mesh.h"
#include "model/snow_model.h"
#include "gui/snow_app.h"
#include "gui/snow_dialog.h"
#include "tools/snow_camera.h"
#include "tools/snow_arcball.h"
// #endif

#ifdef SNOW_MODULE_FFMPEG
#include "media/snow_media.h"
#endif

#undef main