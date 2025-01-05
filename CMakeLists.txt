# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.19)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(nvc_fuzz)


#
# NVC
#

if(NOT DEFINED NVC_COMMIT_SHA OR NVC_COMMIT_SHA STREQUAL "")
  message(FATAL_ERROR "NVC_COMMIT_SHA must be defined and non-empty.")
endif()

include(ExternalProject)

find_program(MAKE_EXECUTABLE NAMES make gmake mingw32-make REQUIRED)
string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)

ExternalProject_Add(
  nvc
  GIT_REPOSITORY https://github.com/nickg/nvc.git
  GIT_TAG ${NVC_COMMIT_SHA}
  PATCH_COMMAND
    ${CMAKE_COMMAND}
      -DBIN=<BINARY_DIR> -DSRC=<SOURCE_DIR>
      -DLST=${CMAKE_CURRENT_LIST_DIR}
      -P ${CMAKE_CURRENT_LIST_DIR}/patch.cmake
  CONFIGURE_HANDLED_BY_BUILD true
  CONFIGURE_COMMAND
    <SOURCE_DIR>/configure --disable-llvm --disable-lto --enable-debug
      "CC=${CMAKE_C_COMPILER}"
      "CFLAGS=-fsanitize=fuzzer-no-link ${CMAKE_C_FLAGS_${BUILD_TYPE_UPPER}} ${CMAKE_C_FLAGS}"
      "LDFLAGS=-fsanitize=fuzzer-no-link ${CMAKE_EXE_LINKER_FLAGS}"
  BUILD_COMMAND
    ASAN_OPTIONS=detect_leaks=0
    ${MAKE_EXECUTABLE} -j
  INSTALL_COMMAND ""
  TEST_COMMAND ""
  BYPRODUCTS
    <BINARY_DIR>/lib/libnvc.a
    <BINARY_DIR>/lib/libfastlz.a
    <BINARY_DIR>/lib/libcpustate.a
    <BINARY_DIR>/lib/libgnulib.a
    <BINARY_DIR>/lib/libsha1.a
)

ExternalProject_Add_Step(
  nvc
  autogen
  WORKING_DIRECTORY <SOURCE_DIR>
  COMMAND ./autogen.sh
  DEPENDEES download
  DEPENDERS configure
)

ExternalProject_Get_property(nvc BINARY_DIR)
set(NVC_LIBS_PATH ${BINARY_DIR}/lib)

add_library(nvc_all INTERFACE)
foreach(lib nvc fastlz cpustate gnulib sha1)
  add_library(nvc_${lib} STATIC IMPORTED GLOBAL)
  set_target_properties(nvc_${lib} PROPERTIES
    IMPORTED_LOCATION ${NVC_LIBS_PATH}/lib${lib}.a
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
  )
  add_dependencies(nvc_${lib} nvc)
  target_link_libraries(nvc_all INTERFACE nvc_${lib})
endforeach()

ExternalProject_Get_property(nvc SOURCE_DIR)
target_include_directories(nvc_all INTERFACE ${SOURCE_DIR}/..)
target_include_directories(nvc_all INTERFACE ${SOURCE_DIR}/src)

include(FindPkgConfig)
add_library(nvc_deps INTERFACE)
foreach(dep libdw libffi libzstd)
  pkg_check_modules(DEP REQUIRED ${dep})
  target_link_libraries(nvc_all INTERFACE ${DEP_LIBRARIES})
  target_include_directories(nvc_all INTERFACE ${DEP_INCLUDE_DIRS})
endforeach()


#
# Our actual Fuzz Tester
#

add_executable(
  fuzzer
  fuzzer.c
  tester.c
)
target_include_directories(fuzzer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fuzzer PRIVATE nvc_all)

target_compile_options(fuzzer PRIVATE -fsanitize=fuzzer)
target_link_options(fuzzer PRIVATE -fsanitize=fuzzer)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
  target_link_libraries(fuzzer PUBLIC ${MATH_LIBRARY})
endif()

add_custom_target(run
  COMMAND ${CMAKE_COMMAND} -E make_directory ../corpus
  COMMAND ${CMAKE_COMMAND} -E rm -rf ../corpus_old
  COMMAND ${CMAKE_COMMAND} -E rename ../corpus ../corpus_old
  COMMAND ./fuzzer
    -only_ascii=1 -reload=0 -max_total_time=3600
    -create_missing_dirs=1 -artifact_prefix=../crash/
    ../corpus ../corpus_old ../seed 2>&1 | tee run.log
  DEPENDS fuzzer
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
)
