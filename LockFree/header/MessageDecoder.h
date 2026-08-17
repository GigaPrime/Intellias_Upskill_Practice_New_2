#pragma once

#include "Message.h"
#include "MessageEncoder.h"

class MessageDecoder 
{
private:
    static std::string readField(const std::array<std::byte, MESSAGE_SIZE>& encodedMessage,
                                 std::size_t offset,
                                 std::size_t length);

public:
    Message decode(const std::array<std::byte, MESSAGE_SIZE>& encodedMessage) const;
};

