add_executable(rtranslator_tests test_main.cpp)
target_include_directories(rtranslator_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(rtranslator_tests PRIVATE
  GTest::gtest_main
  nlohmann_json::nlohmann_json
  Qt5::Core Qt5::WebSockets Qt5::Network
)
include(GoogleTest)
gtest_discover_tests(rtranslator_tests)
