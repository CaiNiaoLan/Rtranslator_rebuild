#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QNetworkProxyFactory>
#include "bridge/CdpClient.h"

TEST(CdpClient, initially_not_connected) {
    CdpClient client;
    EXPECT_FALSE(client.isConnected());
}

TEST(CdpClient, connectToDebugger_emits_disconnected_for_invalid_host) {
    QNetworkProxyFactory::setUseSystemConfiguration(false);
    CdpClient client;
    QSignalSpy spy(&client, &CdpClient::disconnected);
    client.connectToDebugger("127.0.0.1", 19999);
    spy.wait(5000);
    EXPECT_GE(spy.count(), 1);
}

TEST(CdpClient, injectScript_noop_when_not_connected) {
    CdpClient client;
    client.injectScript("console.log('test');");
}
