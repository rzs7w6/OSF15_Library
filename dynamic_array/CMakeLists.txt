cmake_minimum_required (VERSION 2.8.0)
project (dynamic_array)

set(CMAKE_C_FLAGS "${CMAKE_CPP_FLAGS} -std=c99 -Wall -Werror")
set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_library(dynamic_array SHARED src/dynamic_array.c)

install(TARGETS dynamic_array DESTINATION lib)
install(FILES include/dynamic_array.h DESTINATION include)

# http://www.cmake.org/Wiki/CMakeEmulateMakeCheck

set(CMAKE_BUILD_TYPE Debug)
enable_testing()
add_executable(dyn_array_tester test/tester.c)
add_test(tester dyn_array_tester)

# testing like this just doesn't work well with what I have
# since it's not written for CTest/Check
# but it's enough for a flat did it work or not sort of thing.
# probably not helpful in the slightest, but it's at least
# something to refrence in the future
