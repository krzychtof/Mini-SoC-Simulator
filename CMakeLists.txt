cmake_minimum_required(VERSION 3.21)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE
        "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain")
endif()

project(MiniSocSimulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_library(project_warnings INTERFACE)
if(MSVC)
    target_compile_options(project_warnings INTERFACE /W3 /permissive-)
else()
    target_compile_options(project_warnings INTERFACE -Wall -Wextra)
endif()

find_package(SystemCLanguage CONFIG REQUIRED)

add_library(soc_headers INTERFACE)
target_include_directories(soc_headers INTERFACE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(soc_headers INTERFACE SystemC::systemc project_warnings)

add_executable(soc_sim src/top.cpp)
target_link_libraries(soc_sim PRIVATE soc_headers)

find_package(GTest CONFIG REQUIRED)
enable_testing()

add_executable(soc_tests tests/test_main.cpp)
target_link_libraries(soc_tests PRIVATE soc_headers GTest::gtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(soc_tests)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")