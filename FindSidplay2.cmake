# - Try to find sidplay2 
# Once done this will define
#
# SIDPLAY2_FOUND - system has libsidplay2
# SIDPLAY2_INCLUDE_DIRS - the libsidplay2 include directory
# SIDPLAY2_LIBRARIES - The libsidplay2 libraries

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_check_modules (SIDPLAY2 libsidplay2)
  list(APPEND SIDPLAY2_INCLUDE_DIRS ${SIDPLAY2_INCLUDEDIR})
endif()

if(NOT SIDPLAY2_FOUND)
  find_path(SIDPLAY2_INCLUDE_DIRS sidplay/sidplay2.h)
  find_library(SIDPLAY2_LIBRARIES sidplay2)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sidplay2 DEFAULT_MSG SIDPLAY2_INCLUDE_DIRS SIDPLAY2_LIBRARIES)

mark_as_advanced(SIDPLAY2_INCLUDE_DIRS SIDPLAY2_LIBRARIES)
