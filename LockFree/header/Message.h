#pragma once

#include <string>

static constexpr std::size_t ID_SIZE = 8;
static constexpr std::size_t HEADER_SIZE = 16;
static constexpr std::size_t PAYLOAD_SIZE = 128;
static constexpr std::size_t MESSAGE_SIZE = ID_SIZE + HEADER_SIZE + PAYLOAD_SIZE;

class Message 
{
public:
    Message();
    explicit Message(std::string header);
    Message(std::string header, std::string payload);

    static Message fromDecodedFields(std::string id, std::string header, std::string payload);

    const std::string& getId() const;
    const std::string& getHeader() const;
    const std::string& getPayload() const;
    std::string getMessage() const;

private:
    Message(std::string id, std::string header, std::string payload, bool decoded);

    static std::string generateMessageId();
    static void validateAsciiAndLength(const std::string& value, std::size_t maxLength, const std::string& fieldName);

    std::string id_;
    std::string header_;
    std::string payload_;
};

