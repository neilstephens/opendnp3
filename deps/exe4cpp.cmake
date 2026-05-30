include(FetchContent)

FetchContent_Declare(
    exe4cpp
    URL      https://github.com/neilstephens/exe4cpp/archive/430a09508828777a2367c57a83f8ea80c19ec979.zip
    URL_HASH SHA1=2a9d93b467742253c837c9d850c7e02b3cc09dba
)
FetchContent_MakeAvailable(exe4cpp)

if(NOT TARGET exe4cpp)
    add_library(exe4cpp INTERFACE IMPORTED)
    target_include_directories(exe4cpp INTERFACE ${exe4cpp_SOURCE_DIR}/src)
    target_compile_features(exe4cpp INTERFACE cxx_std_14)
endif()
