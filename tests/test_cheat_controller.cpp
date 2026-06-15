#include <gtest/gtest.h>
#include <QSignalSpy>
#include "core/CheatController.h"

TEST(CheatController, readState_sends_read_command) {
    CheatController ctrl;
    std::string captured;
    ctrl.setSendHandler([&](const QString& json) { captured = json.toStdString(); });
    ctrl.readState("party.gold");
    auto j = nlohmann::json::parse(captured);
    EXPECT_EQ(j["type"], "read");
    EXPECT_EQ(j["path"], "party.gold");
}

TEST(CheatController, writeState_sends_write_command) {
    CheatController ctrl;
    std::string captured;
    ctrl.setSendHandler([&](const QString& json) { captured = json.toStdString(); });
    ctrl.writeState("party.gold", 99999);
    auto j = nlohmann::json::parse(captured);
    EXPECT_EQ(j["type"], "write");
    EXPECT_EQ(j["value"], 99999);
}

TEST(CheatController, battleVictory_sends_correct_type) {
    CheatController ctrl;
    std::string captured;
    ctrl.setSendHandler([&](const QString& json) { captured = json.toStdString(); });
    ctrl.battleVictory();
    auto j = nlohmann::json::parse(captured);
    EXPECT_EQ(j["type"], "battle_victory");
}

TEST(CheatController, addItem_sends_item_info) {
    CheatController ctrl;
    std::string captured;
    ctrl.setSendHandler([&](const QString& json) { captured = json.toStdString(); });
    ctrl.addItem(10, "item", 5);
    auto j = nlohmann::json::parse(captured);
    EXPECT_EQ(j["type"], "add_item");
    EXPECT_EQ(j["count"], 5);
}

TEST(CheatController, handleMessage_state_triggers_signal) {
    CheatController ctrl;
    QSignalSpy spy(&ctrl, &CheatController::stateUpdated);
    ctrl.handleMessage(R"({"type":"state","path":"party.gold","value":999})");
    EXPECT_EQ(spy.count(), 1);
}

TEST(CheatController, handleMessage_error_triggers_signal) {
    CheatController ctrl;
    QSignalSpy spy(&ctrl, &CheatController::operationFailed);
    ctrl.handleMessage(R"({"type":"error","cmd":"read","reason":"bad path"})");
    EXPECT_EQ(spy.count(), 1);
}
