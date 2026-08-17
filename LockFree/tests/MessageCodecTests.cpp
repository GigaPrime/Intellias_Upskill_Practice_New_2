#include "Message.h"
#include "MessageDecoder.h"
#include "MessageEncoder.h"

#include <gtest/gtest.h>

TEST(MessageCodecTests, GeneratesEightCharacterId)
{
    const Message message("header", "payload");

    EXPECT_EQ(Message::ID_SIZE, message.getId().size());
}

TEST(MessageCodecTests, RejectsOversizedFields)
{
    EXPECT_THROW(Message(std::string(17, 'h')), std::invalid_argument);
    EXPECT_THROW(Message("header", std::string(129, 'p')), std::invalid_argument);
}

TEST(MessageCodecTests, RejectsNonAsciiFields)
{
    EXPECT_THROW(Message("snowman-\xE2\x98\x83"), std::invalid_argument);
}

TEST(MessageCodecTests, RoundTripsThroughFixedBinaryCodec)
{
    const Message original("hdr", "payload");
    const MessageEncoder encoder;
    const MessageDecoder decoder;

    const auto encoded = encoder.encode(original);
    const auto decoded = decoder.decode(encoded);

    EXPECT_EQ(original.getId(), decoded.getId());
    EXPECT_EQ(original.getHeader(), decoded.getHeader());
    EXPECT_EQ(original.getPayload(), decoded.getPayload());
    EXPECT_EQ(original.getMessage(), decoded.getMessage());
}

