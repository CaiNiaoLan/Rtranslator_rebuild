#include <gtest/gtest.h>
#include <QSignalSpy>
#include "bridge/WebSocketServer.h"

TEST(WebSocketServer, starts_on_auto_port) {
    WebSocketServer server;
    EXPECT_TRUE(server.start(0));
    EXPECT_GT(server.port(), 0);
    EXPECT_TRUE(server.isListening());
    server.stop();
}

TEST(WebSocketServer, stop_closes_server) {
    WebSocketServer server;
    server.start(0);
    server.stop();
    EXPECT_FALSE(server.isListening());
}
