cmake_minimum_required(VERSION 3.20)
project(RTranslator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 5.15 REQUIRED COMPONENTS Core Quick WebSockets Network)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE
  "$ENV{CONDA_PREFIX}/lib/site-packages/nlohmann_json/include"
)
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)

include(FetchContent)
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
    GIT_SHALLOW TRUE
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
  enable_testing()
  add_subdirectory(tests)
endif()

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)

add_executable(RTranslator ${SRC_FILES} qml/qml.qrc)

target_include_directories(RTranslator PRIVATE src)
target_link_libraries(RTranslator PRIVATE
  Qt5::Core Qt5::Quick Qt5::WebSockets Qt5::Network
  nlohmann_json::nlohmann_json
)

set_target_properties(RTranslator PROPERTIES
  WIN32_EXECUTABLE TRUE
)
