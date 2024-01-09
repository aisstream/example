cmake_minimum_required(VERSION 3.20)

project(AisStream
    VERSION 1.0.0
    LANGUAGES C CXX
    HOMEPAGE_URL "https://aisstream.io/"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_AUTOMOC ON)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# Example with beast
find_package(Boost)
find_package(OpenSSL COMPONENTS Crypto SSL)
if(OPENSSL_FOUND AND Boost_FOUND)
    add_executable(example_beast beast.cpp)
    target_link_libraries(example_beast PUBLIC OpenSSL::SSL)
    target_include_directories(example_beast PUBLIC ${Boost_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}/..)
    target_link_libraries(example_beast ${Boost_LIBRARIES})
    set_target_properties(example_beast PROPERTIES AUTOMOC OFF)
    if(MSVC)
        target_compile_options(example_beast PRIVATE /bigobj)
    endif()
endif()

# Example with Qt
find_package(Qt6 COMPONENTS Core WebSockets)
if(Qt6_FOUND)
    qt_add_executable(example_qt qt.cpp)
    set_target_properties(example_qt PROPERTIES WIN32_EXECUTABLE FALSE MACOSX_BUNDLE FALSE)
    target_link_libraries(example_qt PUBLIC Qt6::WebSockets)
endif()
