cmake_minimum_required(VERSION 3.5)
project(tree)

set(CMAKE_CXX_STANDARD 17)

# Compiler flags for the different targets
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC -Wall -Wextra -fno-omit-frame-pointer -march=native -Wno-unknown-pragmas")
# Only use address sanitizer in debug builds
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")

# Make sure we use O3 optimization level instead of O2
string(REPLACE "-O2" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -O3")
string(REPLACE "-O2" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3")

include(lib/papi.cmake)

include_directories(
  third_party)

add_executable(high_level high_level.cpp)
target_include_directories(high_level PRIVATE $<BUILD_INTERFACE:${PAPI_INCLUDE_DIR}>)
target_link_libraries(high_level ${PAPI_LIBRARY_PATH})
add_dependencies(high_level papi)
