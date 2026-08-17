#include "MessageEncoder.h"

#include <algorithm>

std::array<std::byte, MESSAGE_SIZE> MessageEncoder::encode(const Message& message) const
{
    std::array<std::byte, MESSAGE_SIZE> encodedMessage{};

    writeField(encodedMessage, 0, ID_SIZE, message.getId());
    writeField(encodedMessage, ID_SIZE, HEADER_SIZE, message.getHeader());
    writeField(encodedMessage, ID_SIZE + HEADER_SIZE, PAYLOAD_SIZE, message.getPayload());

    return encodedMessage;
}

void MessageEncoder::writeField(std::array<std::byte, MESSAGE_SIZE>& encodedMessage,
                                std::size_t offset,
                                std::size_t length,
                                const std::string& value)
{
    for (std::size_t index = 0; index < length; ++index) {
        encodedMessage[offset + index] = std::byte{0};
    }

    for (std::size_t index = 0; index < value.size(); ++index) {
        encodedMessage[offset + index] = static_cast<std::byte>(value[index]);
    }
}

