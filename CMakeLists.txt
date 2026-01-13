cmake_minimum_required(VERSION "3.30")

project(Specs)
set(CMAKE_CXX_STANDARD 26)

#file(GLOB_RECURSE SPECS_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ixx")

add_library(Specs STATIC) 

target_include_directories(Specs PRIVATE "src")
target_include_directories(Specs PRIVATE "include")

file(GLOB_RECURSE PUBLIC_MODULES CONFIGURE_DEPENDS "public/*.ixx")
file(GLOB_RECURSE PRIVATE_MODULES CONFIGURE_DEPENDS "src/*.ixx")
file(GLOB_RECURSE PRIVATE_CPP CONFIGURE_DEPENDS "src/*.cpp")

target_sources(Specs
    PUBLIC
        FILE_SET cxx_modules_public TYPE CXX_MODULES FILES
            ${PUBLIC_MODULES}
    PRIVATE
        FILE_SET cxx_modules_private TYPE CXX_MODULES FILES
            ${PRIVATE_MODULES}
    ${PRIVATE_CPP}
)

target_compile_options(Specs PRIVATE -Wall)