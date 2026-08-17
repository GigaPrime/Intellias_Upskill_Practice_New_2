#include "Buffer.h"

#include <stdexcept>

Buffer::Buffer(std::size_t capacity) : messageCapacity_(capacity), internalCapacity_(capacity + 1), buffer_(internalCapacity_)
{
    if (capacity == 0) 
    {
        throw std::invalid_argument("capacity must be greater than zero");
    }
}

bool Buffer::store(const Message& message)
{
    const std::size_t tail = tail_.load(std::memory_order_relaxed); // memory_order_acquire
    const std::size_t head = head_.load(std::memory_order_acquire);
    const std::size_t nextTail = increment(tail);

    if (nextTail == head) 
    {
        return false;
    }

    buffer_[tail] = message;
    tail_.store(nextTail, std::memory_order_release);
    return true;
}

bool Buffer::pop(Message& message)
{
    const std::size_t head = head_.load(std::memory_order_relaxed); // memory_order_acquire

    if (head == tail_.load(std::memory_order_acquire)) 
    {
        return false;
    }

    message = buffer_[head];
    head_.store(increment(head), std::memory_order_release);
    return true;
}

bool Buffer::empty() const
{
    return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
}

bool Buffer::full() const
{
    const std::size_t tail = tail_.load(std::memory_order_acquire);
    return increment(tail) == head_.load(std::memory_order_acquire);
}

std::size_t Buffer::capacity() const
{
    return messageCapacity_;
}

std::size_t Buffer::increment(std::size_t index) const
{
    return (index + 1) % internalCapacity_;
}
