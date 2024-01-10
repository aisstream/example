cmake_minimum_required(VERSION 3.20)

project(AisStream
    VERSION 1.0.0
    LANGUAGES C
    HOMEPAGE_URL "https://aisstream.io/"
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_DEBUG_POSTFIX d)
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
endif()

# Example with mongoose
find_package(OpenSSL QUIET COMPONENTS Crypto SSL)
if(OPENSSL_FOUND)
    if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/mongoose.c)
        file(DOWNLOAD https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.c ${CMAKE_CURRENT_BINARY_DIR}/mongoose.c)
    endif()
    if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/mongoose.h)
        file(DOWNLOAD https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.h ${CMAKE_CURRENT_BINARY_DIR}/mongoose.h)
    endif()
    add_executable(example_mg mg.c ${CMAKE_CURRENT_BINARY_DIR}/mongoose.c ${CMAKE_CURRENT_BINARY_DIR}/mongoose.h)
    target_include_directories(example_mg PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(example_mg PUBLIC OpenSSL::SSL)
    target_compile_definitions(example_mg PRIVATE MG_TLS=MG_TLS_OPENSSL)
endif()

# Example with libwebsockets
find_package(libwebsockets QUIET)
if(libwebsockets_FOUND)
    include(LwsCheckRequirements)
    set(requirements 1)
    require_lws_config(LWS_ROLE_WS 1 requirements)
    require_lws_config(LWS_WITH_CLIENT 1 requirements)

    if (requirements)
        add_executable(example_lws lws.c)
        target_link_libraries(example_lws PUBLIC websockets)
    endif()
endif()
