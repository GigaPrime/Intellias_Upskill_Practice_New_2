#pragma once

#include "Buffer.h"
#include "Message.h"
#include "MessageEncoder.h"
#include "UdpSocket.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <utility>

struct StreamerStats {
    std::size_t streamedCount{};
};

class UdpStreamer 
{
public:
    UdpStreamer(Buffer& buffer,
                std::string destinationHost,
                std::uint16_t destinationPort,
                std::chrono::milliseconds idleSleep = std::chrono::milliseconds(1))
        : buffer_(buffer)
        , destinationHost_(std::move(destinationHost))
        , destinationPort_(destinationPort)
        , idleSleep_(idleSleep)
    {
    }

    ~UdpStreamer()
    {
        stop();
    }

    UdpStreamer(const UdpStreamer&) = delete;
    UdpStreamer& operator=(const UdpStreamer&) = delete;

    void start()
    {
        bool expected = false;
        if (!running_.compare_exchange_strong(expected, true)) {
            return;
        }

        worker_ = std::thread(&UdpStreamer::run, this);
    }

    void stop()
    {
        running_.store(false, std::memory_order_release);
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    StreamerStats getStats() const
    {
        return StreamerStats{streamedCount_.load(std::memory_order_acquire)};
    }

private:
    void run()
    {
        UdpSocket socket;
        socket.openSender();
        MessageEncoder encoder;

        while (running_.load(std::memory_order_acquire)) {
            Message message;
            if (!buffer_.pop(message)) {
                std::this_thread::sleep_for(idleSleep_);
                continue;
            }

            const auto encodedMessage = encoder.encode(message);
            if (socket.sendTo(encodedMessage.data(), encodedMessage.size(), destinationHost_, destinationPort_)) {
                streamedCount_.fetch_add(1, std::memory_order_acq_rel);
            }
        }
    }

    Buffer& buffer_;
    std::string destinationHost_;
    std::uint16_t destinationPort_{};
    std::chrono::milliseconds idleSleep_;
    std::atomic<bool> running_{false};
    std::thread worker_;
    std::atomic<std::size_t> streamedCount_{0};
};

