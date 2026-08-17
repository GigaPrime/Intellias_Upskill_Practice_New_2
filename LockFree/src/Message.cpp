#include "Message.h"

#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>

Message::Message()
    : id_(generateMessageId())
{
}

Message::Message(std::string header)
    : id_(generateMessageId())
    , header_(std::move(header))
{
    validateAsciiAndLength(header_, HEADER_SIZE, "header");
}

Message::Message(std::string header, std::string payload)
    : id_(generateMessageId())
    , header_(std::move(header))
    , payload_(std::move(payload))
{
    validateAsciiAndLength(header_, HEADER_SIZE, "header");
    validateAsciiAndLength(payload_, PAYLOAD_SIZE, "payload");
}

Message Message::fromDecodedFields(std::string id, std::string header, std::string payload)
{
    return Message(std::move(id), std::move(header), std::move(payload), true);
}

const std::string& Message::getId() const
{
    return id_;
}

const std::string& Message::getHeader() const
{
    return header_;
}

const std::string& Message::getPayload() const
{
    return payload_;
}

std::string Message::getMessage() const
{
    return id_ + header_ + payload_;
}

Message::Message(std::string id, std::string header, std::string payload, bool)
    : id_(std::move(id))
    , header_(std::move(header))
    , payload_(std::move(payload))
{
    validateAsciiAndLength(id_, ID_SIZE, "id");
    validateAsciiAndLength(header_, HEADER_SIZE, "header");
    validateAsciiAndLength(payload_, PAYLOAD_SIZE, "payload");

    if (id_.size() != ID_SIZE) {
        throw std::invalid_argument("id must be exactly 8 ASCII characters");
    }
}

std::string Message::generateMessageId()
{
    const auto now = std::chrono::system_clock::now();
    const auto epochSeconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    const auto timestampSuffix = static_cast<int>(epochSeconds % 10000);

    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(0, 9999);

    std::ostringstream stream;
    stream << std::setw(4) << std::setfill('0') << timestampSuffix
           << std::setw(4) << std::setfill('0') << distribution(generator);
    return stream.str();
}

void Message::validateAsciiAndLength(const std::string& value, std::size_t maxLength, const std::string& fieldName)
{
    if (value.size() > maxLength) {
        throw std::invalid_argument(fieldName + " exceeds fixed field length");
    }

    for (const unsigned char character : value) {
        if (character > 0x7F) {
            throw std::invalid_argument(fieldName + " must contain ASCII characters only");
        }
    }
}

