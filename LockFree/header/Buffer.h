#pragma once

#include "Message.h"

#include <atomic>
#include <thread>
#include <vector>

class Buffer 
{
private:
    mutable std::atomic<bool> isFree_ = true;
    mutable std::atomic<std::thread::id> threadId_;

    std::size_t increment(std::size_t index) const;

    std::size_t messageCapacity_{};
    std::size_t internalCapacity_{};
    std::vector<Message> buffer_;
	alignas(64) std::atomic<std::size_t> head_{ 0 };        // alignas here is used to avoid false sharing between head_ and tail_
	alignas(64) std::atomic<std::size_t> tail_{ 0 };        // since modern CPUs cache lines are typically 64 bytes, aligning these atomic variables to 64 bytes                                                     // helps to ensure that they do not share a cache line, which can lead to performance degradation due to false sharing.

    bool checkAcquired() const;

public:
	explicit Buffer(std::size_t capacity);                  // capacity in Messages, not in bytes

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    bool acquire();
    bool release();

	bool store(const Message& message);                     // dumb name but it does not conflict like push() does with std::queue
    bool pop(Message& message);
    bool empty() const;
    bool full() const;
    std::size_t capacity() const;

};
