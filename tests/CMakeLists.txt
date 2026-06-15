add_executable(rtranslator_tests
    test_main.cpp
    test_protocol.cpp
    ../src/bridge/Protocol.cpp
    test_json_helper.cpp
    ../src/util/JsonHelper.cpp
    test_rpgmv_adapter.cpp
    ../src/adapter/RpgMVAdapter.cpp
    test_translation_engine.cpp
    ../src/core/TranslationEngine.cpp
    test_websocket_server.cpp
    ../src/bridge/WebSocketServer.cpp
    test_cdp_client.cpp
    ../src/bridge/CdpClient.cpp
    test_cheat_controller.cpp
    ../src/core/CheatController.cpp
    test_integration.cpp
)
target_include_directories(rtranslator_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(rtranslator_tests PRIVATE TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/tests/test_data")
target_link_libraries(rtranslator_tests PRIVATE
  GTest::gtest_main
  nlohmann_json::nlohmann_json
  Qt5::Core Qt5::WebSockets Qt5::Network Qt5::Test
)
include(GoogleTest)
gtest_discover_tests(rtranslator_tests)
