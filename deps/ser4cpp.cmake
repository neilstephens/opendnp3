include(FetchContent)

FetchContent_Declare(
    ser4cpp
    URL      https://github.com/neilstephens/ser4cpp/archive/e316f8397bea296ef4872ee4121aacdc33ed7e8f.zip
    URL_HASH SHA1=8f93b52426a2d309eb6baf76d741462b85ae1748
)
FetchContent_MakeAvailable(ser4cpp)

if(NOT TARGET ser4cpp)
    add_library(ser4cpp INTERFACE IMPORTED)
    target_include_directories(ser4cpp INTERFACE ${ser4cpp_SOURCE_DIR}/src)
    target_compile_features(ser4cpp INTERFACE cxx_std_14)
endif()
