#pragma once

#include "Buffer.h"
#include "Message.h"

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct ProducerStats
{
    std::size_t generatedCount{};
    std::size_t producedCount{};
    std::size_t droppedCount{};
    std::vector<std::string> generatedIds;
    std::vector<std::string> droppedIds;
};

class Producer 
{
private:
    void run();

    Buffer& buffer_;
    const std::size_t retryCount_ = 5;
    std::chrono::milliseconds minDelay_;
    std::chrono::milliseconds maxDelay_;
    std::atomic<bool> running_{false};
    std::thread worker_;
    mutable std::mutex mutex_;
    std::vector<std::string> generatedIds_;
    std::vector<std::string> droppedIds_;
    std::atomic<std::size_t> generatedCount_{0};
    std::atomic<std::size_t> producedCount_{0};
    std::atomic<std::size_t> droppedCount_{0};

public:
    explicit Producer(Buffer& buffer,
        std::chrono::milliseconds minDelay = std::chrono::milliseconds(10),
        std::chrono::milliseconds maxDelay = std::chrono::milliseconds(100));

    ~Producer();

    Producer(const Producer&) = delete;
    Producer& operator=(const Producer&) = delete;

    void start();
    void stop();
    ProducerStats getStats() const;

};

