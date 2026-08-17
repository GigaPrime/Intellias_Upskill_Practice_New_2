#pragma once

#include "Message.h"
#include "MessageDecoder.h"
#include "SqliteBatchWriter.h"
#include "UdpSocket.h"

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct ConsumerStats 
{
    std::size_t consumedCount{};
    std::vector<std::string> consumedIds;
};

class Consumer 
{
private:
    void run();
    void recordConsumedId(const std::string& id);

    std::uint16_t listenPort_{};
    std::string databasePath_{};
    std::size_t batchSize_{};   
    std::chrono::milliseconds idleSleep_;
    std::atomic<bool> running_{false};
    std::thread worker_;
    mutable std::mutex mutex_;
    std::vector<std::string> consumedIds_;
    std::atomic<std::size_t> consumedCount_{0};

public:
    Consumer(const std::uint16_t listenPort,
        const std::string& databasePath,
        const std::size_t batchSize,
        const std::chrono::milliseconds idleSleep = std::chrono::milliseconds(1));

    ~Consumer();

    Consumer(const Consumer&) = delete;
    Consumer& operator=(const Consumer&) = delete;

    void start();
    void stop();
    ConsumerStats getStats() const;
};

