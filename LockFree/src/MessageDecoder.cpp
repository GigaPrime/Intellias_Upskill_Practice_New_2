#include "MessageDecoder.h"

#include <algorithm>

Message MessageDecoder::decode(const std::array<std::byte, MESSAGE_SIZE>& encodedMessage) const
{
    return Message::fromDecodedFields(
        readField(encodedMessage, 0, ID_SIZE),
        readField(encodedMessage, ID_SIZE, HEADER_SIZE),
        readField(encodedMessage, ID_SIZE + HEADER_SIZE, PAYLOAD_SIZE));
}

std::string MessageDecoder::readField(const std::array<std::byte, MESSAGE_SIZE>& encodedMessage,
                                      std::size_t offset,
                                      std::size_t length)
{
    std::string decodedMessage;
    decodedMessage.reserve(length);

    for (std::size_t index = 0; index < length; ++index) 
    {
        const char letter  = static_cast<char>(encodedMessage[offset + index]);
        if (letter == '\0') 
        {
            break;
        }
        decodedMessage.push_back(letter);
    }

    return decodedMessage;
}

