#include "Producer.h"

#include <random>
#include <stdexcept>

Producer::Producer(Buffer& buffer,
                   std::chrono::milliseconds minDelay,
                   std::chrono::milliseconds maxDelay)
    : buffer_(buffer)
    , minDelay_(minDelay)
    , maxDelay_(maxDelay)
{
    if (minDelay_ > maxDelay_) 
    {
        throw std::invalid_argument("minDelay cannot be greater than maxDelay");
    }

	generatedIds_.reserve(buffer_.capacity());
	droppedIds_.reserve(buffer_.capacity());
}

Producer::~Producer()
{
    stop();
}

void Producer::start()
{
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true))
    {
        return;
    }

    worker_ = std::thread([this]() { run(); });
}

void Producer::stop()
{
    running_.store(false, std::memory_order_release);
    if (worker_.joinable()) 
    {
        worker_.join();
    }
}

ProducerStats Producer::getStats() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return ProducerStats
    {
        generatedCount_.load(std::memory_order_acquire),
        producedCount_.load(std::memory_order_acquire),
        droppedCount_.load(std::memory_order_acquire),
        generatedIds_,
        droppedIds_
    };
}

void Producer::run()
{
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());
    std::uniform_int_distribution<int> delayDistribution(
        static_cast<int>(minDelay_.count()),
        static_cast<int>(maxDelay_.count()));
    std::size_t sequence = 0;

    while (running_.load(std::memory_order_acquire)) 
    {
        Message message("header-" + std::to_string(sequence % 1000000000), "payload-" + std::to_string(sequence));
        generatedCount_.fetch_add(1, std::memory_order_acq_rel);

        std::size_t retrys = 0;
        bool stored = false;
        while (retrys < retryCount_ && !stored) // retry logic
        {
            if (buffer_.acquire())
            {
                stored = buffer_.store(message);
                ++retrys;
                buffer_.release();
            }
            if (!stored)
            {
                std::this_thread::yield();
            }
        }

        {
            std::lock_guard lock(mutex_);
            if (stored)
            {
                producedCount_.fetch_add(1, std::memory_order_acq_rel);
                generatedIds_.push_back(message.getId());
            }
            else
            {
                droppedCount_.fetch_add(1, std::memory_order_acq_rel);
                droppedIds_.push_back(message.getId());
            }
        }

        ++sequence;
        std::this_thread::sleep_for(std::chrono::milliseconds(delayDistribution(generator)));
    }
}
