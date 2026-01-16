cmake_minimum_required(VERSION "3.30")

project(Specs)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")


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
)

target_sources(Specs PRIVATE ${PRIVATE_CPP})

target_compile_options(Specs PRIVATE -Wall)

add_executable(SpecsTest tests/main.cpp)
target_link_libraries(SpecsTest PRIVATE Specs)

enable_testing()
add_test(NAME SpecsTest COMMAND SpecsTest)
