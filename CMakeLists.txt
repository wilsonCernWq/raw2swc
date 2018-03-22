cmake_minimum_required(VERSION 2.8)
project(raw2swc)

set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp helper.h)
add_executable(raw2swc ${SOURCE_FILES})

# find_package(OpenMP)
# target_compile_options(raw2swc 
#   PUBLIC "$<$<CONFIG:DEBUG>:${OpenMP_C_FLAGS} -parallel>")
# target_compile_options(raw2swc 
#   PUBLIC "$<$<CONFIG:RELEASE>:${OpenMP_C_FLAGS} -parallel>")
