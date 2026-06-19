#include <gtest/gtest.h>
#include "core/TranslationEngine.h"
#include "adapter/RpgMVAdapter.h"
#include "bridge/Protocol.h"
#include "bridge/WebSocketServer.h"
#include "bridge/CdpClient.h"
#include "core/CheatController.h"

TEST(Integration, translationMap_to_protocol_roundtrip) {
    auto engine = std::make_unique<TranslationEngine>(std::make_unique<RpgMVAdapter>());
    ASSERT_TRUE(engine->loadTranslation(QString::fromUtf8(TEST_DATA_DIR "/sample_trans.json")));
    Protocol::InitCommand init;
    init.translationMap = engine->translationMap();
    auto json = init.toJson();
    EXPECT_EQ(json["map"]["hello"], "nihao");
    EXPECT_EQ(json["map"]["world"], "shijie");
}

TEST(Integration, cheat_write_and_parse_roundtrip) {
    CheatController ctrl;
    std::string lastSent;
    ctrl.setSendHandler([&](const QString& json) { lastSent = json.toStdString(); });
    ctrl.writeState("party.gold", 99999);
    auto j = nlohmann::json::parse(lastSent);
    EXPECT_EQ(j["type"], "write");
    EXPECT_EQ(j["value"], 99999);
    std::string stateMsg = R"({"type":"state","path":"party.gold","value":99999})";
    ctrl.handleMessage(QString::fromStdString(stateMsg));
}

TEST(Integration, websocket_server_starts_and_stops) {
    WebSocketServer server;
    ASSERT_TRUE(server.start(0));
    EXPECT_GT(server.port(), 0);
    EXPECT_TRUE(server.isListening());
    server.stop();
    EXPECT_FALSE(server.isListening());
}

TEST(Integration, batch_write_serializes_multiple_ops) {
    CheatController ctrl;
    std::string captured;
    ctrl.setSendHandler([&](const QString& json) { captured = json.toStdString(); });
    ctrl.batchWrite({{"party.gold", 1000}, {"switches.5", true}});
    auto j = nlohmann::json::parse(captured);
    EXPECT_EQ(j["type"], "batch");
    EXPECT_EQ(j["ops"].size(), 2);
}

TEST(Integration, translation_engine_detect_game) {
    auto engine = std::make_unique<TranslationEngine>(std::make_unique<RpgMVAdapter>());
    EXPECT_FALSE(engine->detectGame(QString::fromStdString("nonexistent/path")));
}
