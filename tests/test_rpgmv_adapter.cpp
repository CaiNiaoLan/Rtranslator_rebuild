#include <gtest/gtest.h>
#include "adapter/RpgMVAdapter.h"
#include <QDir>
#include <QFile>
#include <QTextStream>

class RpgMVAdapterTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir = "tests/test_data/mv_game/www/data";
        QDir().mkpath(QString::fromStdString(testDir));
    }

    void writeJson(const std::string& filename, const std::string& content) {
        QFile file(QString::fromStdString(testDir + "/" + filename));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << QString::fromStdString(content);
        file.close();
    }

    void TearDown() override {
        QDir("tests/test_data").removeRecursively();
    }

    std::string testDir;
};

TEST_F(RpgMVAdapterTest, detect_returns_false_when_no_system_json) {
    RpgMVAdapter adapter;
    EXPECT_FALSE(adapter.detect("tests/test_data/mv_game"));
}

TEST_F(RpgMVAdapterTest, detect_returns_true_when_system_json_exists) {
    writeJson("System.json", "{}");
    RpgMVAdapter adapter;
    EXPECT_TRUE(adapter.detect("tests/test_data/mv_game"));
}

TEST_F(RpgMVAdapterTest, extractText_parses_actors) {
    writeJson("System.json", "{}");
    writeJson("Actors.json", R"([{"id":1,"name":"Hero","nickname":"H","profile":"Brave","note":"Tag"}])");

    RpgMVAdapter adapter;
    auto entries = adapter.extractText("tests/test_data/mv_game");
    EXPECT_GE(entries.size(), 4);
}

TEST_F(RpgMVAdapterTest, extractText_parses_items) {
    writeJson("System.json", "{}");
    writeJson("Items.json", R"([{"id":1,"name":"Potion","description":"Heals 50HP","note":"Common"}])");

    RpgMVAdapter adapter;
    auto entries = adapter.extractText("tests/test_data/mv_game");
    EXPECT_GE(entries.size(), 3);
}

TEST_F(RpgMVAdapterTest, extractText_parses_system_terms) {
    writeJson("System.json", R"({
        "gameTitle":"MyRPG",
        "currencyUnit":"G",
        "terms":{"basic":["HP","MP"],"commands":["Fight","Escape"]}
    })");

    RpgMVAdapter adapter;
    auto entries = adapter.extractText("tests/test_data/mv_game");
    EXPECT_GE(entries.size(), 5);
}
