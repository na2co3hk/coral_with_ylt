cmake_minimum_required(VERSION 3.22.1)
project(coral_with_ylt)

set(CMAKE_CXX_STANDARD 20)

#C++20
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
    #-ftree-slp-vectorize with coroutine cause link error. disable it util gcc fix.
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-tree-slp-vectorize")
endif()

#yalanting/include
include_directories(yalantinglibs/include)

#yalanting/ylt
include_directories(yalantinglibs/include/ylt)

#yalanting/third-party
include_directories(yalantinglibs/include/ylt/thirdparty)

include_directories(src)

add_executable(coral_with_ylt main.cpp)

