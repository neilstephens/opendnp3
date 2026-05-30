include(FetchContent)

FetchContent_Declare(
    asio
    URL      https://github.com/chriskohlhoff/asio/archive/asio-1-38-0.zip
    URL_HASH SHA1=f0c2fe431c5dcb87f9e0c104ef2e33fbb117c4d2
)
FetchContent_MakeAvailable(asio)

if(NOT TARGET asio)
    find_package(Threads)

    add_library(asio INTERFACE IMPORTED)
    target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/include)
    target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
    target_compile_features(asio INTERFACE cxx_std_11)
    target_link_libraries(asio INTERFACE Threads::Threads)

    if(WIN32)
        target_link_libraries(asio INTERFACE ws2_32 wsock32) # Link to Winsock
        target_compile_definitions(asio INTERFACE _WIN32_WINNT=0x0601) # Windows 7 and up
    endif()
endif()
