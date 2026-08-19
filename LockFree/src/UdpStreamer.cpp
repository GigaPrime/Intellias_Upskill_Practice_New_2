#include "UdpStreamer.h"

#include "Message.h"
#include "MessageEncoder.h"
#include "UdpSocket.h"

#include <utility>

UdpStreamer::UdpStreamer(Buffer& buffer,
                         std::string destinationHost,
                         std::uint16_t destinationPort,
                         std::chrono::milliseconds idleSleep)
    : buffer_(buffer)
    , destinationHost_(std::move(destinationHost))
    , destinationPort_(destinationPort)
    , idleSleep_(idleSleep) {}

UdpStreamer::~UdpStreamer()
{
    stop();
}

void UdpStreamer::start()
{
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true)) 
    {
        return;
    }

    worker_ = std::thread(&UdpStreamer::run, this);
}

void UdpStreamer::stop()
{
    running_.store(false, std::memory_order_release);
    if (worker_.joinable()) 
    {
        worker_.join();
    }
}

StreamerStats UdpStreamer::getStats() const
{
    return StreamerStats{streamedCount_.load(std::memory_order_acquire)};
}

void UdpStreamer::run()
{
    UdpSocket socket;
    socket.openSender();
    MessageEncoder encoder;

    while (running_.load(std::memory_order_acquire)) 
    {
        Message message;
        bool popped = false;

        if (buffer_.acquire())
        {
            popped = buffer_.pop(message);
            buffer_.release();
        }

        if (!popped)
        {
            std::this_thread::sleep_for(idleSleep_);
            continue;
        }

        const auto encodedMessage = encoder.encode(message);

        if (socket.sendTo(encodedMessage.data(), encodedMessage.size(), destinationHost_, destinationPort_))
        {
            streamedCount_.fetch_add(1, std::memory_order_relaxed);
        }
    }
}
