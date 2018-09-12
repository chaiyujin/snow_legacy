if (WIN32)
    set(SDL2_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/../third_party/SDL2-2.0.8/include")
    set(SDL2_LIBRARIES_DIRS "${PROJECT_SOURCE_DIR}/../third_party/SDL2-2.0.8/lib/x64")
    # set dll, cache
    set(SDL2_WIN32_DLLS "${PROJECT_SOURCE_DIR}/../third_party/SDL2-2.0.8/lib/x64/SDL2.dll" CACHE INTERNAL "DIRS")

    include_directories("${SDL2_INCLUDE_DIRS}")
    link_directories("${SDL2_LIBRARIES_DIRS}")
else ()
    # find package
    find_package(SDL2)
endif (WIN32)
