#include "Consumer.h"
#include "Message.h"
#include "MessageEncoder.h"
#include "UdpSocket.h"

#include <gtest/gtest.h>
#include <sqlite3.h>

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <string>
#include <thread>

std::size_t countRows(const std::filesystem::path& databasePath)
{
    sqlite3* database = nullptr;
    EXPECT_EQ(SQLITE_OK, sqlite3_open(databasePath.string().c_str(), &database));

    sqlite3_stmt* statement = nullptr;
    EXPECT_EQ(SQLITE_OK, sqlite3_prepare_v2(database, "SELECT COUNT(*) FROM messages;", -1, &statement, nullptr));
    EXPECT_EQ(SQLITE_ROW, sqlite3_step(statement));
    const auto rows = static_cast<std::size_t>(sqlite3_column_int64(statement, 0));
    sqlite3_finalize(statement);
    sqlite3_close(database);
    return rows;
}

TEST(ConsumerIntegrationTests, ReceivesUdpMessagesAndWritesSqliteBatch)
{
    const auto databasePath = std::filesystem::temp_directory_path() / "spsc_consumer_integration.db";
    std::filesystem::remove(databasePath);

    constexpr std::uint16_t port = 41117;
    Consumer consumer(port, databasePath.string(), 2);
    consumer.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    UdpSocket sender;
    sender.openSender();
    MessageEncoder encoder;

    const Message first("h1", "p1");
    const Message second("h2", "p2");
    const auto firstEncoded = encoder.encode(first);
    const auto secondEncoded = encoder.encode(second);

    EXPECT_TRUE(sender.sendTo(firstEncoded.data(), firstEncoded.size(), "127.0.0.1", port));
    EXPECT_TRUE(sender.sendTo(secondEncoded.data(), secondEncoded.size(), "127.0.0.1", port));

    for (int attempt = 0; attempt < 50; ++attempt) {
        if (consumer.getStats().consumedCount >= 2) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    consumer.stop();

    const auto stats = consumer.getStats();
    EXPECT_GE(stats.consumedCount, 2U);
    EXPECT_GE(countRows(databasePath), 2U);

    std::filesystem::remove(databasePath);
}
