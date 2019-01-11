if (WIN32)
    # tell the package position
    set(ASSIMP_INCLUDE_DIRS   "${PROJECT_SOURCE_DIR}/../third_party/assimp-3.1.1-win-binaries/include"          )
    set(ASSIMP_LIBRARIES_DIRS "${PROJECT_SOURCE_DIR}/../third_party/assimp-3.1.1-win-binaries/lib64"            )
    # set dll, cache
    set(ASSIMP_WIN32_DLLS     "${PROJECT_SOURCE_DIR}/../third_party/assimp-3.1.1-win-binaries/bin64/assimp.dll" )

    mark_as_advanced(
        ASSIMP_INCLUDE_DIRS
        ASSIMP_LIBRARIES_DIRS
        ASSIMP_WIN32_DLLS
    )

else ()
    # find package
    find_package(assimp)
endif (WIN32)
