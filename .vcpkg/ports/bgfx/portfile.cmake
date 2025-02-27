# For a list of common variables see https://github.com/microsoft/vcpkg/blob/master/docs/maintainers/vcpkg_common_definitions.md

# Download source packages
# (bgfx requires bx and bimg source for building)

vcpkg_from_github(OUT_SOURCE_PATH BX_SOURCE_DIR
    REPO "julianxhokaxhiu/bx"
    HEAD_REF master
    REF d52f04216cb9af8ec6198aa06c9917f0fe0e5ae0
    SHA512 8f61d07238a32ae693c18668256ddda1c3fe68dbe93c131f2aab777c948c3e0fe3fdeb2d987cb10be22b6d5dd038aa4e2d8193893aef82cca83f1a433c33407a
)

vcpkg_from_github(OUT_SOURCE_PATH BIMG_SOURCE_DIR
    REPO "julianxhokaxhiu/bimg"
    HEAD_REF master
    REF 9e4d2b167ffb4ecf4dba625dee59f6be7cf2587a
    SHA512 dfb8b8e87282d520347f587eab6cb7074c71f4379d42b0e96aec8b30c57f5b320dd6f2ef2ba3b0a8807e3b022f6a9aaebf629f2629458868bcb09a9c1be24e65
)

vcpkg_from_github(OUT_SOURCE_PATH SOURCE_DIR
    REPO "julianxhokaxhiu/bgfx"
    HEAD_REF master
    REF 8a32a010f6a356b0014a4c3a01bfe7f7ca9f591f
    SHA512 e27234a20346bdf361531fc8a7a477a75af8307d5e6e139a9b4f4889ab8e2e1616c947a66d869517f47973042bf2ffe4b4e8e71f483d895b80d821889ad4bbc1
)

# Move bx source inside bgfx source tree
set(BX_DIR ${SOURCE_DIR}/.bx)
file(RENAME ${BX_SOURCE_DIR} "${BX_DIR}")
set(ENV{BX_DIR} ${BX_DIR})

# Move bimg source inside bgfx source tree
set(BIMG_DIR ${SOURCE_DIR}/.bimg)
file(RENAME ${BIMG_SOURCE_DIR} "${BIMG_DIR}")
set(ENV{BIMG_DIR} ${BIMG_DIR})

# Set custom BGFX configuration
set(ENV{BGFX_CONFIG} "DEBUG=1:PREFER_DISCRETE_GPU=0:DYNAMIC_INDEX_BUFFER_SIZE=10485760:DYNAMIC_VERTEX_BUFFER_SIZE=31457280")

# Set up GENie (custom project generator)
set(GENIE_OPTIONS --with-tools)

if(VCPKG_CRT_LINKAGE STREQUAL dynamic)
    set(GENIE_OPTIONS ${GENIE_OPTIONS} --with-dynamic-runtime)
endif()
if(VCPKG_LIBRARY_LINKAGE STREQUAL "dynamic")
    set(GENIE_OPTIONS ${GENIE_OPTIONS} --with-shared-lib)
endif()

if(VCPKG_TARGET_ARCHITECTURE STREQUAL x86)
    set(GENIE_OPTIONS ${GENIE_OPTIONS} --platform=x32)
elseif(VCPKG_TARGET_ARCHITECTURE STREQUAL x64)
    set(GENIE_OPTIONS ${GENIE_OPTIONS} --platform=x64)
elseif(VCPKG_TARGET_ARCHITECTURE STREQUAL arm OR VCPKG_TARGET_ARCHITECTURE STREQUAL arm64)
    set(GENIE_OPTIONS ${GENIE_OPTIONS} --platform=ARM)
else()
    message(WARNING "Architecture may be not supported: ${VCPKG_TARGET_ARCHITECTURE}")
    set(GENIE_OPTIONS ${GENIE_OPTIONS} --platform=${VCPKG_TARGET_ARCHITECTURE})
endif()

if(TARGET_TRIPLET MATCHES osx)
    set(GENIE_OPTIONS ${GENIE_OPTIONS} --os=macosx)
elseif(TARGET_TRIPLET MATCHES linux)
    set(GENIE_OPTIONS ${GENIE_OPTIONS} --os=linux)
elseif(TARGET_TRIPLET MATCHES windows)
    set(GENIE_OPTIONS ${GENIE_OPTIONS} --os=windows)
elseif(TARGET_TRIPLET MATCHES uwp)
    set(GENIE_OPTIONS ${GENIE_OPTIONS} --vs=winstore100)
endif()

# GENie does not allow cmake+msvc, so we use msbuild in windows
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    if(VCPKG_PLATFORM_TOOLSET STREQUAL "v140")
        set(GENIE_ACTION vs2015)
    elseif(VCPKG_PLATFORM_TOOLSET STREQUAL "v141")
        set(GENIE_ACTION vs2017)
    elseif(VCPKG_PLATFORM_TOOLSET STREQUAL "v142")
        set(GENIE_ACTION vs2019)
    elseif(VCPKG_PLATFORM_TOOLSET STREQUAL "v143")
        set(GENIE_ACTION vs2022)
    else()
        message(FATAL_ERROR "Unsupported Visual Studio toolset: ${VCPKG_PLATFORM_TOOLSET}")
    endif()
    set(PROJ_FOLDER ${GENIE_ACTION})
    if(TARGET_TRIPLET MATCHES uwp)
        set(PROJ_FOLDER ${PROJ_FOLDER}-winstore100)
    endif()
else()
    set(GENIE_ACTION cmake)
    set(PROJ_FOLDER ${GENIE_ACTION})
endif()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(GENIE "${BX_DIR}/tools/bin/windows/genie.exe")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(GENIE "${BX_DIR}/tools/bin/darwin/genie")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(GENIE "${BX_DIR}/tools/bin/linux/genie")
else()
    message(FATAL_ERROR "Unsupported host platform: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

# Run GENie

vcpkg_execute_required_process(
    COMMAND ${GENIE} ${GENIE_OPTIONS} ${GENIE_ACTION}
    WORKING_DIRECTORY "${SOURCE_DIR}"
    LOGNAME "genie-${TARGET_TRIPLET}"
)

if(GENIE_ACTION STREQUAL cmake)
    if(VCPKG_LIBRARY_LINKAGE STREQUAL "dynamic")
        set(PROJ bgfx-shared-lib)
    else()
        set(PROJ bgfx)
    endif()
    vcpkg_configure_cmake(
        SOURCE_PATH "${SOURCE_DIR}/.build/projects/${PROJ_FOLDER}"
        PREFER_NINJA
        OPTIONS_RELEASE -DCMAKE_BUILD_TYPE=Release
        OPTIONS_DEBUG -DCMAKE_BUILD_TYPE=Debug
    )
    vcpkg_install_cmake(TARGET ${PROJ}/all)
    file(INSTALL "${SOURCE_DIR}/include/bgfx" DESTINATION "${CURRENT_PACKAGES_DIR}/include")
    file(GLOB instfiles
        "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/${PROJ}/*.a"
        "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel/${PROJ}/*.so"
    )
    file(INSTALL ${instfiles} DESTINATION "${CURRENT_PACKAGES_DIR}/lib")
    file(GLOB instfiles
        "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg/${PROJ}/*.a"
        "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg/${PROJ}/*.so"
    )
    file(INSTALL ${instfiles} DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib")
    file(INSTALL "${SOURCE_DIR}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME "copyright")
else()
    vcpkg_install_msbuild(
        SOURCE_PATH "${SOURCE_DIR}"
        PROJECT_SUBPATH ".build/projects/${PROJ_FOLDER}/bgfx.sln"
        LICENSE_SUBPATH "LICENSE"
        INCLUDES_SUBPATH "include"
    )
    # Install shader include file
    file(INSTALL "${SOURCE_DIR}/src/bgfx_shader.sh" DESTINATION "${CURRENT_PACKAGES_DIR}/include/bgfx" )
    # Remove redundant files
    foreach(a bx bimg bimg_decode bimg_encode)
        foreach(b Debug Release)
            foreach(c lib pdb)
                if(b STREQUAL Debug)
                    file(REMOVE "${CURRENT_PACKAGES_DIR}/debug/lib/${a}${b}.${c}")
                else()
                    file(REMOVE "${CURRENT_PACKAGES_DIR}/lib/${a}${b}.${c}")
                endif()
            endforeach()
        endforeach()
    endforeach()
endif()

configure_file(${CMAKE_CURRENT_LIST_DIR}/FindBGFX.cmake.in ${CURRENT_PACKAGES_DIR}/share/${PORT}/FindBGFX.cmake @ONLY)
file(COPY ${CMAKE_CURRENT_LIST_DIR}/vcpkg-cmake-wrapper.cmake DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT})
