#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "snow" for configuration "Debug"
set_property(TARGET snow APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(snow PROPERTIES
  IMPORTED_LOCATION_DEBUG "/home/chaiyujin/Documents/GitHub/snow/libsnow/x86_64/Debug/lib/libsnow.so"
  IMPORTED_SONAME_DEBUG "libsnow.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS snow )
list(APPEND _IMPORT_CHECK_FILES_FOR_snow "/home/chaiyujin/Documents/GitHub/snow/libsnow/x86_64/Debug/lib/libsnow.so" )

# Import target "snow-core" for configuration "Debug"
set_property(TARGET snow-core APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(snow-core PROPERTIES
  IMPORTED_LOCATION_DEBUG "/home/chaiyujin/Documents/GitHub/snow/libsnow/x86_64/Debug/lib/libsnow-core.so"
  IMPORTED_SONAME_DEBUG "libsnow-core.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS snow-core )
list(APPEND _IMPORT_CHECK_FILES_FOR_snow-core "/home/chaiyujin/Documents/GitHub/snow/libsnow/x86_64/Debug/lib/libsnow-core.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
