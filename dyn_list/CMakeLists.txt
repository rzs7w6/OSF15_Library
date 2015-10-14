cmake_minimum_required (VERSION 2.8)
project(dyn_list)

# Look, I know it's a bad name, especially considering the
# list-who-shall-not-be-named is WAY more dynamic than this
# but we really don't want to clobber a list implementation
# what, do you want me to just append UUIDs to every header?
# That would be fun.

set(CMAKE_C_FLAGS "-std=c99 -Wall -Werror")
set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_library(${PROJECT_NAME} SHARED src/${PROJECT_NAME}.c)
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES include/${PROJECT_NAME}.h DESTINATION include)

set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include
	CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)




# tester gibberish


# http://www.cmake.org/Wiki/CMakeEmulateMakeCheck

set(CMAKE_BUILD_TYPE Debug)
enable_testing()

# This MUST be checked on the machine being installed on
# This list WILL NOT work if this fails
add_executeable(dyn_list_vital_check test/vital.c)
add_test(teser dyn_list_vital_check)

add_executable(dyn_list_tester test/tester.c)
add_test(tester dyn_list_tester)

# testing like this just doesn't work well with what I have
# since it's not written for CTest/Check
# but it's enough for a flat did it work or not sort of thing.
# probably not helpful in the slightest, but it's at least
# something to refrence in the future
