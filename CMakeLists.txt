cmake_minimum_required(VERSION "3.30")

project(Specs)
set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_EXTENSIONS OFF)

add_library(Specs STATIC) 

add_subdirectory("external/unordered_dense")

target_include_directories(Specs PRIVATE "src")

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "src/*.cpp")

target_sources(Specs PRIVATE ${SRC})

target_compile_options(Specs PRIVATE -Wall)

add_executable(SpecsTest tests/main.cpp)
target_link_libraries(SpecsTest PRIVATE Specs)

target_include_directories(Specs PUBLIC "include")

target_include_directories(Specs PUBLIC SYSTEM "external/ctti/include")
target_include_directories(Specs PUBLIC SYSTEM "external/boost/include")
target_include_directories(Specs PUBLIC SYSTEM "external/unordered_dense/include")

#target_compile_options(SpecsTest PRIVATE -stdlib=libstdc++)
#target_link_options(SpecsTest PRIVATE -stdlib=libstdc++)

enable_testing()
add_test(NAME SpecsTest COMMAND SpecsTest)
