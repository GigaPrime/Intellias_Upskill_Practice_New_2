#include "Buffer.h"

#include <stdexcept>

Buffer::Buffer(std::size_t capacity) : messageCapacity_(capacity), internalCapacity_(capacity + 1), buffer_(internalCapacity_)
{
    if (capacity == 0) 
    {
        throw std::invalid_argument("capacity must be greater than zero");
    }
}

bool Buffer::acquire() // it actually never returns false :-/
{
    bool acquired = true;
    while (!isFree_.compare_exchange_strong(acquired, false, std::memory_order_acquire)) // potential ABA here?
    {
        acquired = true;
        std::this_thread::yield();  // slicing time of the thred in favor of another thread ready to run
                                    // it is still a context switch
        // isFree.wait(false) - futex underneeth available with C++20
    }

    threadId_ = std::this_thread::get_id();
    return acquired;
}

bool Buffer::release()
{
    if (!checkAcquired()) 
    {
        return false;
    }

    threadId_.store(std::thread::id{});

    bool expected = false;
    return isFree_.compare_exchange_strong( expected, true, std::memory_order_release);
}

bool Buffer::checkAcquired() const
{
    return !isFree_.load(std::memory_order_acquire) &&
            threadId_.load(std::memory_order_acquire) == std::this_thread::get_id();
}

bool Buffer::store(const Message& message)
{  
    if (!checkAcquired())
    {
        return false;
    }

    const std::size_t tail = tail_.load(std::memory_order_acquire);
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
    if (!checkAcquired())
    {
        return false;
    }

    const std::size_t head = head_.load(std::memory_order_acquire);

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

std::size_t Buffer::increment(std::size_t index) const // No need to check if acquired, this is a private method.
{
    return (index + 1) % internalCapacity_;
}
