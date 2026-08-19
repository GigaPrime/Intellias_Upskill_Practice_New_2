#pragma once

#include "Buffer.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <thread>

struct StreamerStats 
{
    std::size_t streamedCount{};
};

class UdpStreamer 
{
private:
    void run();

    Buffer& buffer_;
    std::string destinationHost_;
    std::uint16_t destinationPort_{};
    std::chrono::milliseconds idleSleep_;
    std::atomic<bool> running_{ false };
    std::thread worker_;
    std::atomic<std::size_t> streamedCount_{ 0 };

public:
    UdpStreamer(Buffer& buffer,
                std::string destinationHost,
                std::uint16_t destinationPort,
                std::chrono::milliseconds idleSleep = std::chrono::milliseconds(1));

    ~UdpStreamer();

    UdpStreamer(const UdpStreamer&) = delete;
    UdpStreamer& operator=(const UdpStreamer&) = delete;

    void start();
    void stop();
    StreamerStats getStats() const;
};

