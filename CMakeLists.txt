cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
include(FetchContent)

if(ESP_PLATFORM)
  file(GLOB_RECURSE SRC src/*.c)
  idf_component_register(SRCS ${SRC} INCLUDE_DIRS include REQUIRES driver)
  set_target_properties(${COMPONENT_LIB} PROPERTIES PREFIX "")
  set_target_properties(
    ${COMPONENT_LIB} PROPERTIES OUTPUT_NAME
                                ${CMAKE_STATIC_LIBRARY_PREFIX}${COMPONENT_LIB})
  target_link_libraries(${COMPONENT_LIB} PUBLIC MDU)
endif()

FetchContent_Declare(
  CMakeModules
  GIT_REPOSITORY "https://github.com/ZIMO-Elektronik/CMakeModules"
  GIT_TAG v0.9.1
  SOURCE_DIR ${CMAKE_BINARY_DIR}/CMakeModules)
FetchContent_MakeAvailable(CMakeModules)

version_from_git()
project(
  MDU
  VERSION ${VERSION_FROM_GIT}
  LANGUAGES CXX)

option(MDU_UNITY_BUILD "Combine source files into single batch" ON)
set(MDU_RX_MIN_PREAMBLE_BITS
    10u
    CACHE STRING "Minimum number of preambel bits of decoder")
set(MDU_TX_MIN_PREAMBLE_BITS
    14u
    CACHE STRING "Minimum number of preambel bits of command station")
set(MDU_TX_MIN_ACKREQ_BITS
    10u
    CACHE STRING "Minimum number of ackreq bits of command station")

file(GLOB_RECURSE SRC src/*.cpp)
add_library(MDU STATIC ${SRC})
add_library(MDU::MDU ALIAS MDU)

target_compile_features(MDU PUBLIC cxx_std_23)

target_compile_definitions(
  MDU
  PUBLIC MDU_RX_MIN_PREAMBLE_BITS=${MDU_RX_MIN_PREAMBLE_BITS}
         MDU_TX_MIN_PREAMBLE_BITS=${MDU_TX_MIN_PREAMBLE_BITS}
         MDU_TX_MIN_ACKREQ_BITS=${MDU_TX_MIN_ACKREQ_BITS})

if(PROJECT_IS_TOP_LEVEL)
  target_include_directories(
    MDU
    INTERFACE include
    PRIVATE include/mdu)
else()
  target_include_directories(
    MDU SYSTEM
    INTERFACE include
    PRIVATE include/mdu)
endif()

target_common_warnings(MDU PRIVATE)

if(MDU_UNITY_BUILD)
  target_unity_build(MDU)
endif()

if(NOT TARGET Microsoft.GSL::GSL)
  cpmaddpackage("gh:microsoft/GSL@4.0.0")
endif()

if(NOT TARGET Salsa20::Salsa20)
  cpmaddpackage("gh:ZIMO-Elektronik/Salsa20@0.3.0")
endif()

if(NOT TARGET ZTL::ZTL)
  cpmaddpackage("gh:ZIMO-Elektronik/ZTL@0.19.0")
endif()

target_link_libraries(MDU PUBLIC Microsoft.GSL::GSL Salsa20::Salsa20 ZTL::ZTL)

if(PROJECT_IS_TOP_LEVEL)
  include(CTest)
  add_subdirectory(examples)
  file(
    DOWNLOAD
    "https://github.com/ZIMO-Elektronik/.clang-format/raw/master/.clang-format"
    ${CMAKE_CURRENT_LIST_DIR}/.clang-format)
  file(GLOB_RECURSE SRC examples/*.[ch]pp include/*.[ch]pp src/*.[ch]pp
       tests/*.[ch]pp)
  add_clang_format_target(MDUFormat OPTIONS -i FILES ${SRC})
endif()

if(BUILD_TESTING AND CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
  add_subdirectory(tests)
endif()
