include(FetchContent)

FetchContent_Declare(
    exe4cpp
    URL      https://github.com/neilstephens/exe4cpp/archive/88bb2d3194a17c01c8df110d30ce1338e6b7c0cd.zip
    URL_HASH SHA1=4be0e509d9768015032e1d94cf265d9b3f82bc2b
)
FetchContent_MakeAvailable(exe4cpp)

if(NOT TARGET exe4cpp)
    add_library(exe4cpp INTERFACE IMPORTED)
    target_include_directories(exe4cpp INTERFACE ${exe4cpp_SOURCE_DIR}/src)
    target_compile_features(exe4cpp INTERFACE cxx_std_14)
endif()
