project(audiodecoder.sidplay)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(xbmc REQUIRED)
find_package(Sidplay2)
find_package(Resid)

add_definitions(-D_LINUX -DHAVE_UNIX)

set(SID_SOURCES src/SIDCodec.cpp)

if(SIDPLAY2_FOUND)
  list(APPEND DEPLIBS ${SIDPLAY2_LIBRARIES}
                      ${RESID_LIBRARIES})
  include_directories(${XBMC_INCLUDE_DIR}
                      ${RESID_INCLUDE_DIRS}
                      ${SIDPLAY2_INCLUDE_DIRS})
else()
  include_directories(${XBMC_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}/lib/libsidplay2/
                    ${PROJECT_SOURCE_DIR}/lib/libsidplay2/builders/resid-builder/include/sidplay/builders
                    ${PROJECT_SOURCE_DIR}/lib/libsidplay2/libsidplay/include/sidplay
                    ${PROJECT_SOURCE_DIR}/lib/libsidplay2/resid
                    ${PROJECT_SOURCE_DIR}/lib/libsidplay2/libsidplay/include)

  list(APPEND SOURCES
              lib/libsidplay2/builders/resid-builder/src/resid-builder.cpp
              lib/libsidplay2/builders/resid-builder/src/resid.cpp
              lib/libsidplay2/resid/sid.cc
              lib/libsidplay2/libsidplay/src/config.cpp
              lib/libsidplay2/libsidplay/src/event.cpp
              lib/libsidplay2/libsidplay/src/mixer.cpp
              lib/libsidplay2/libsidplay/src/player.cpp
              lib/libsidplay2/libsidplay/src/psiddrv.cpp
              lib/libsidplay2/libsidplay/src/reloc65.c
              lib/libsidplay2/libsidplay/src/sidplay2.cpp
              lib/libsidplay2/libsidplay/src/mos6510/mos6510.cpp
              lib/libsidplay2/libsidplay/src/mos6526/mos6526.cpp
              lib/libsidplay2/libsidplay/src/sid6526/sid6526.cpp
              lib/libsidplay2/libsidplay/src/mos656x/mos656x.cpp
              lib/libsidplay2/libsidplay/src/sidtune/IconInfo.cpp
              lib/libsidplay2/libsidplay/src/sidtune/InfoFile.cpp
              lib/libsidplay2/libsidplay/src/sidtune/MUS.cpp
              lib/libsidplay2/libsidplay/src/sidtune/p00.cpp
              lib/libsidplay2/libsidplay/src/sidtune/PP20.cpp
              lib/libsidplay2/libsidplay/src/sidtune/prg.cpp
              lib/libsidplay2/libsidplay/src/sidtune/PSID.cpp
              lib/libsidplay2/libsidplay/src/sidtune/SidTune.cpp
              lib/libsidplay2/libsidplay/src/sidtune/SidTuneTools.cpp
              lib/libsidplay2/libsidplay/src/xsid/xsid.cpp
              lib/libsidplay2/resid/envelope.cc
              lib/libsidplay2/resid/extfilt.cc
              lib/libsidplay2/resid/filter.cc
              lib/libsidplay2/resid/pot.cc
              lib/libsidplay2/resid/version.cc
              lib/libsidplay2/resid/voice.cc
              lib/libsidplay2/resid/wave.cc
              lib/libsidplay2/resid/wave6581__ST.cc
              lib/libsidplay2/resid/wave6581_P_T.cc
              lib/libsidplay2/resid/wave6581_PS_.cc
              lib/libsidplay2/resid/wave6581_PST.cc
              lib/libsidplay2/resid/wave8580__ST.cc
              lib/libsidplay2/resid/wave8580_P_T.cc
              lib/libsidplay2/resid/wave8580_PS_.cc
              lib/libsidplay2/resid/wave8580_PST.cc)
endif()

build_addon(audiodecoder.sidplay SID DEPLIBS)
