cmake_minimum_required(VERSION 3.20)

project(AisStream
    VERSION 1.0.0
    LANGUAGES C
    HOMEPAGE_URL "https://aisstream.io/"
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_VISIBILITY_PRESET default)
set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(CMAKE_VISIBILITY_INLINES_HIDDEN OFF)

find_package(libwebsockets REQUIRED)
include(LwsCheckRequirements)
set(requirements 1)
require_lws_config(LWS_ROLE_WS 1 requirements)
require_lws_config(LWS_WITH_CLIENT 1 requirements)

if (requirements)
    add_executable(example_c main.c)
    target_link_libraries(example_c PUBLIC websockets)
endif()
