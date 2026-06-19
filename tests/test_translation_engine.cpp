#include <gtest/gtest.h>
#include "core/TranslationEngine.h"
#include "adapter/RpgMVAdapter.h"
#include <QDir>
#include <QFile>
#include <QTextStream>

class TranslationEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_unique<TranslationEngine>(std::make_unique<RpgMVAdapter>());
        testDir = "tests/test_data/trans_engine";
        QDir().mkpath(QString::fromStdString(testDir + "/www/data"));
    }
    void writeJson(const std::string& filename, const std::string& content) {
        QFile file(QString::fromStdString(testDir + "/www/data/" + filename));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << QString::fromStdString(content);
        file.close();
    }
    void TearDown() override {
        QDir("tests/test_data/trans_engine").removeRecursively();
    }
    std::unique_ptr<TranslationEngine> engine;
    std::string testDir;
};

TEST_F(TranslationEngineTest, detectGame_returns_true_for_valid_game) {
    writeJson("System.json", "{}");
    EXPECT_TRUE(engine->detectGame(QString::fromStdString(testDir)));
}

TEST_F(TranslationEngineTest, detectGame_returns_false_for_empty_dir) {
    EXPECT_FALSE(engine->detectGame(QString::fromStdString(testDir)));
}

TEST_F(TranslationEngineTest, extractFromGame_returns_entries) {
    writeJson("System.json", R"({"gameTitle":"Test","currencyUnit":"G","terms":{}})");
    writeJson("Actors.json", R"([{"id":1,"name":"Hero","nickname":"H","profile":"P","note":"N"}])");
    auto entries = engine->extractFromGame(QString::fromStdString(testDir));
    EXPECT_GT(entries.size(), 0);
}

TEST_F(TranslationEngineTest, buildWhitelist_contains_extracted_strings) {
    std::vector<TranslationEntry> entries = {
        {"id1","src","ctx","f","hello",""},
        {"id2","src","ctx","f","world",""}
    };
    auto wl = engine->buildWhitelist(entries);
    EXPECT_NE(wl.find("hello"), wl.end());
    EXPECT_NE(wl.find("world"), wl.end());
    EXPECT_EQ(wl.size(), 2);
}

TEST_F(TranslationEngineTest, loadTranslation_returns_false_for_missing_file) {
    EXPECT_FALSE(engine->loadTranslation(QString::fromStdString("tests/test_data/nonexistent.json")));
}

TEST_F(TranslationEngineTest, translate_returns_translation_when_found) {
    EXPECT_TRUE(engine->loadTranslation(QString::fromUtf8(TEST_DATA_DIR "/sample_trans.json")));
    auto result = engine->translate("hello");
    EXPECT_EQ(result, "nihao");
}

TEST_F(TranslationEngineTest, translate_returns_empty_when_not_found) {
    EXPECT_TRUE(engine->loadTranslation(QString::fromUtf8(TEST_DATA_DIR "/sample_trans.json")));
    auto result = engine->translate("unknown_key");
    EXPECT_TRUE(result.empty());
}
