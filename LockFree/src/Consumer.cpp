#include "Consumer.h"

#include <array>
#include <stdexcept>

Consumer::Consumer(const std::uint16_t listenPort,
                   const std::string& databasePath,
                   const std::size_t batchSize,
                   const std::chrono::milliseconds idleSleep)
    : listenPort_(listenPort)
    , databasePath_(databasePath)
    , batchSize_(batchSize)
    , idleSleep_(idleSleep)
{
}

Consumer::~Consumer()
{
    stop();
}

void Consumer::start()
{
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true)) {
        return;
    }

    worker_ = std::thread(&Consumer::run, this);
}

void Consumer::stop()
{
    running_.store(false, std::memory_order_release);
    if (worker_.joinable()) {
        worker_.join();
    }
}

ConsumerStats Consumer::getStats() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return ConsumerStats{
        consumedCount_.load(std::memory_order_acquire),
        consumedIds_
    };
}

void Consumer::run()
{
    UdpSocket socket;
    socket.openReceiver(listenPort_);

    MessageDecoder decoder;
    SqliteBatchWriter writer(databasePath_, batchSize_);
    std::array<std::byte, MESSAGE_SIZE> encodedMessage{};

    while (running_.load(std::memory_order_acquire)) {
        const int bytesRead = socket.receive(encodedMessage.data(), encodedMessage.size());

        if (bytesRead == 0) {
            std::this_thread::sleep_for(idleSleep_);
            continue;
        }

        if (bytesRead < 0 || static_cast<std::size_t>(bytesRead) != encodedMessage.size()) {
            continue;
        }

        Message message = decoder.decode(encodedMessage);
        writer.add(message);
        recordConsumedId(message.getId());
    }

    writer.flush();
}

void Consumer::recordConsumedId(const std::string& id)
{
    consumedCount_.fetch_add(1, std::memory_order_acq_rel);
    std::lock_guard<std::mutex> lock(mutex_);
    consumedIds_.push_back(id);
}

