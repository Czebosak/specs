cmake_minimum_required(VERSION "3.30")

project(Specs)
set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_EXTENSIONS OFF)

add_library(Specs STATIC) 

add_subdirectory("external/unordered_dense")

target_include_directories(Specs PRIVATE "src")

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

target_include_directories(Specs PRIVATE "external/ctti/include")
target_include_directories(Specs PRIVATE "external/boost/include")
target_include_directories(Specs PRIVATE "external/unordered_dense/include")

#target_compile_options(SpecsTest PRIVATE -stdlib=libstdc++)
#target_link_options(SpecsTest PRIVATE -stdlib=libstdc++)

enable_testing()
add_test(NAME SpecsTest COMMAND SpecsTest)
