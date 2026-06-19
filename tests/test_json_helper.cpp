#include <gtest/gtest.h>
#include "util/JsonHelper.h"
#include <QString>

using json = nlohmann::json;

TEST(JsonHelper, loadJsonFile_reads_valid_file) {
    auto opt = JsonHelper::loadJsonFile(QString::fromUtf8(TEST_DATA_DIR "/valid.json"));
    ASSERT_TRUE(opt.has_value());
    EXPECT_EQ((*opt)["key"], "value");
}

TEST(JsonHelper, loadJsonFile_returns_nullopt_for_missing_file) {
    auto opt = JsonHelper::loadJsonFile(QString::fromUtf8(TEST_DATA_DIR "/does_not_exist.json"));
    EXPECT_FALSE(opt.has_value());
}

TEST(JsonHelper, loadJsonFile_returns_nullopt_for_invalid_json) {
    auto opt = JsonHelper::loadJsonFile(QString::fromUtf8(TEST_DATA_DIR "/invalid.json"));
    EXPECT_FALSE(opt.has_value());
}

TEST(JsonHelper, getString_returns_value_when_field_exists) {
    json obj = {{"name", "Hero"}};
    auto val = JsonHelper::getString(obj, "name");
    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(*val, "Hero");
}

TEST(JsonHelper, getString_returns_nullopt_when_field_missing) {
    json obj = {{"name", "Hero"}};
    auto val = JsonHelper::getString(obj, "description");
    EXPECT_FALSE(val.has_value());
}

TEST(JsonHelper, getString_returns_nullopt_for_non_string_type) {
    json obj = {{"count", 42}};
    auto val = JsonHelper::getString(obj, "count");
    EXPECT_FALSE(val.has_value());
}

TEST(JsonHelper, collectStrings_aggregates_from_array_field) {
    json arr = json::array({
        {{"name", "A"}},
        {{"name", "B"}},
        {{"name", "A"}}
    });
    std::vector<std::string> result;
    JsonHelper::collectStrings(arr, "name", result);
    EXPECT_EQ(result.size(), 3);
}

TEST(JsonHelper, collectStringsToSet_dedupicates) {
    json arr = json::array({
        {{"name", "A"}},
        {{"name", "B"}},
        {{"name", "A"}}
    });
    std::unordered_set<std::string> result;
    JsonHelper::collectStringsToSet(arr, "name", result);
    EXPECT_EQ(result.size(), 2);
}
