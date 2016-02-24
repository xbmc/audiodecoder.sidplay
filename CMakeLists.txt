project(audiodecoder.sidplay)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(kodi REQUIRED)
find_package(kodiplatform REQUIRED)
find_package(p8-platform REQUIRED)
find_package(Sidplay2 REQUIRED)
find_package(Resid REQUIRED)

if(WIN32)
  add_definitions(-DHAVE_MSWINDOWS)
else()
  add_definitions(-DHAVE_UNIX)
endif()

set(SID_SOURCES src/SIDCodec.cpp)

list(APPEND DEPLIBS ${p8-platform_LIBRARIES}
                    ${SIDPLAY2_LIBRARIES}
                    ${RESID_LIBRARIES})

include_directories(${kodiplatform_INCLUDE_DIRS}
                    ${p8-platform_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}
                    ${RESID_INCLUDE_DIRS}
                    ${SIDPLAY2_INCLUDE_DIRS})

build_addon(audiodecoder.sidplay SID DEPLIBS)

include(CPack)
