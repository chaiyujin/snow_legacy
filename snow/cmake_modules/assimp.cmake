if (WIN32)
    # tell the package position
    set(ASSIMP_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/../third_party/assimp-3.1.1-win-binaries/include")
    set(ASSIMP_LIBRARIES_DIRS "${PROJECT_SOURCE_DIR}/../third_party/assimp-3.1.1-win-binaries/lib64")
    # set dll, cache
    set(ASSIMP_WIN32_DLLS "${PROJECT_SOURCE_DIR}/../third_party/assimp-3.1.1-win-binaries/bin64/assimp.dll" CACHE INTERNAL "DIRS")

    include_directories("${ASSIMP_INCLUDE_DIRS}")
    link_directories("${ASSIMP_LIBRARIES_DIRS}")
else ()
    # find package
    find_package(assimp)
endif (WIN32)
