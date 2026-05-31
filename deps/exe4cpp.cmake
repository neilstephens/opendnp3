include(FetchContent)

FetchContent_Declare(
    exe4cpp
    URL      https://github.com/neilstephens/exe4cpp/archive/eeb2a4458a74b3ad459f228767fc60a110287446.zip
    URL_HASH SHA1=3d79974b96dc427fe95176b0aeb1df137a0f214d
)
FetchContent_MakeAvailable(exe4cpp)

if(NOT TARGET exe4cpp)
    add_library(exe4cpp INTERFACE IMPORTED)
    target_include_directories(exe4cpp INTERFACE ${exe4cpp_SOURCE_DIR}/src)
    target_compile_features(exe4cpp INTERFACE cxx_std_14)
endif()
