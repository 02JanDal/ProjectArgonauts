cmake_minimum_required(VERSION 3.0)

project(ProjectArgonauts VERSION 0.0.1)
enable_testing()

find_package(Boost 1.58 REQUIRED COMPONENTS filesystem system)

set(CMAKE_CXX_FLAGS "-std=c++14 -Wextra -Wall -Weverything -Wno-c++98-compat -Wno-c++98-compat-local-type-template-args -Wno-c++98-compat-pedantic -Wno-padded -Wno-weak-vtables")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${CMAKE_SOURCE_DIR} /usr/include/c++/5.2.0/)

add_subdirectory(embeddedcpptemplate)
add_subdirectory(util)
add_subdirectory(runtime)
add_subdirectory(tool)
add_subdirectory(example)
add_subdirectory(test)

install(EXPORT Argonauts DESTINATION cmake)
install(FILES runtime/ArgonautsFunctions.cmake DESTINATION cmake)
install(DIRECTORY runtime/ DESTINATION include/runtime FILES_MATCHING PATTERN *.h)
install(DIRECTORY util/ DESTINATION include/util FILES_MATCHING PATTERN *.h)
