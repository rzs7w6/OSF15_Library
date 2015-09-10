cmake_minimum_required (VERSION 2.8.0)
project (bitmap)

set(CMAKE_C_FLAGS "${CMAKE_CPP_FLAGS} -std=c99 -Wall -Werror")
set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_library(bitmap SHARED src/bitmap.c)

install(TARGETS bitmap DESTINATION lib)
install(FILES include/bitmap.h DESTINATION include)

set(CMAKE_BUILD_TYPE Debug)
enable_testing()
add_executable(bitmap_tester test/test.c)
add_test(tester bitmap_tester)
