cmake_minimum_required(VERSION 3.10.0)
project(COLBRA CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MacOS specific library path
# only necessary for users who
# installed OpenMP with brew
if (DEFINED ENV{DARWIN})
  list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/libomp")
endif()

find_package(OpenMP REQUIRED)
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

add_executable(colbra src/main.cpp src/hash.cpp src/map.cpp)

if(OpenMP_CXX_FOUND)
  target_link_libraries(colbra PUBLIC OpenMP::OpenMP_CXX)
else()
  message(FATAL_ERROR "OpenMP not found, required for building colbra.")
endif()

if (OPENSSL_FOUND)
  target_include_directories(colbra PUBLIC ${OPENSSL_INCLUDE_DIR})
  target_link_libraries(colbra PUBLIC OpenSSL::Crypto)
else()
  message(FATAL_ERROR "OpenSSL not found, required for building colbra.")
endif()
