#include <gtest/gtest.h>
#include "bridge/Protocol.h"

using json = nlohmann::json;

TEST(Protocol, InitCommand_serializes_correctly) {
    Protocol::InitCommand cmd;
    cmd.translationMap = {{"hello", "nihao"}, {"world", "shijie"}};
    json j = cmd.toJson();
    EXPECT_EQ(j["type"], "init");
    EXPECT_TRUE(j.contains("map"));
    EXPECT_EQ(j["map"]["hello"], "nihao");
    EXPECT_EQ(j["map"]["world"], "shijie");
}

TEST(Protocol, ReadCommand_serializes_path) {
    Protocol::ReadCommand cmd;
    cmd.path = "party.gold";
    json j = cmd.toJson();
    EXPECT_EQ(j["type"], "read");
    EXPECT_EQ(j["path"], "party.gold");
}

TEST(Protocol, WriteCommand_serializes_path_and_value) {
    Protocol::WriteCommand cmd;
    cmd.path = "party.gold";
    cmd.value = 99999;
    json j = cmd.toJson();
    EXPECT_EQ(j["type"], "write");
    EXPECT_EQ(j["path"], "party.gold");
    EXPECT_EQ(j["value"], 99999);
}

TEST(Protocol, parseStateResponse_extracts_path_and_value) {
    json j = {{"type", "state"}, {"path", "party.gold"}, {"value", 500}};
    auto resp = Protocol::StateResponse::fromJson(j);
    EXPECT_TRUE(resp.has_value());
    EXPECT_EQ(resp->path, "party.gold");
    EXPECT_EQ(resp->value, 500);
}

TEST(Protocol, parseAckResponse_extracts_success) {
    json j = {{"type", "ack"}, {"cmd", "write"}, {"success", true}};
    auto resp = Protocol::AckResponse::fromJson(j);
    EXPECT_TRUE(resp.has_value());
    EXPECT_TRUE(resp->success);
}

TEST(Protocol, parseErrorResponse_extracts_reason) {
    json j = {{"type", "error"}, {"cmd", "read"}, {"reason", "invalid path"}};
    auto resp = Protocol::ErrorResponse::fromJson(j);
    EXPECT_TRUE(resp.has_value());
    EXPECT_EQ(resp->reason, "invalid path");
}

TEST(Protocol, unknown_message_type_returns_nullopt) {
    json j = {{"type", "bogus"}};
    auto state = Protocol::StateResponse::fromJson(j);
    EXPECT_FALSE(state.has_value());
}

TEST(Protocol, BatchCommand_serializes_multiple_ops) {
    Protocol::BatchCommand batch;
    batch.ops.push_back(Protocol::ReadCommand{"party.gold"}.toJson());
    batch.ops.push_back(Protocol::WriteCommand{"switches.5", true}.toJson());
    json j = batch.toJson();
    EXPECT_EQ(j["type"], "batch");
    EXPECT_EQ(j["ops"].size(), 2);
    EXPECT_EQ(j["ops"][0]["type"], "read");
    EXPECT_EQ(j["ops"][1]["type"], "write");
}

TEST(Protocol, BattleVictoryCommand_serializes_empty_payload) {
    Protocol::BattleVictoryCommand cmd;
    json j = cmd.toJson();
    EXPECT_EQ(j["type"], "battle_victory");
}

TEST(Protocol, AddItemCommand_serializes_item_info) {
    Protocol::AddItemCommand cmd;
    cmd.id = 10;
    cmd.itemType = "item";
    cmd.count = 5;
    json j = cmd.toJson();
    EXPECT_EQ(j["type"], "add_item");
    EXPECT_EQ(j["id"], 10);
    EXPECT_EQ(j["itemType"], "item");
    EXPECT_EQ(j["count"], 5);
}

TEST(Protocol, ReloadTransCommand_serializes_new_map) {
    Protocol::ReloadTransCommand cmd;
    cmd.translationMap = {{"a", "b"}};
    json j = cmd.toJson();
    EXPECT_EQ(j["type"], "reload_trans");
    EXPECT_EQ(j["map"]["a"], "b");
}
