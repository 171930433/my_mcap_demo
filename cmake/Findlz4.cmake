# Findlz4.cmake — 为 conda-forge 的 lz4-c 包提供 cmake find_package(lz4) 支持
# mcap_builder 需要 target LZ4::lz4

if(TARGET LZ4::lz4)
  set(lz4_FOUND TRUE)
  return()
endif()

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(_lz4 QUIET liblz4)
endif()

find_path(LZ4_INCLUDE_DIR
  NAMES lz4.h
  HINTS ${_lz4_INCLUDE_DIRS}
)
find_library(LZ4_LIBRARY
  NAMES lz4
  HINTS ${_lz4_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(lz4
  REQUIRED_VARS LZ4_LIBRARY LZ4_INCLUDE_DIR
)

if(lz4_FOUND AND NOT TARGET LZ4::lz4)
  add_library(LZ4::lz4 UNKNOWN IMPORTED)
  set_target_properties(LZ4::lz4 PROPERTIES
    IMPORTED_LOCATION "${LZ4_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${LZ4_INCLUDE_DIR}"
  )
endif()
