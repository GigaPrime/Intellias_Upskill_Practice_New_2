#pragma once

#include "Message.h"

#include <array>
#include <cstddef>

class MessageEncoder 
{
private:
    static void writeField(std::array<std::byte, MESSAGE_SIZE>& encodedMessage,
                           std::size_t offset,
                           std::size_t length,
                           const std::string& value);

public:
    std::array<std::byte, MESSAGE_SIZE> encode(const Message& message) const;
};

