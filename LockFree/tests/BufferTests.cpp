#include "Buffer.h"
#include "Message.h"

#include <gtest/gtest.h>

TEST(BufferTests, StoresAndPopsInFifoOrder)
{
    Buffer buffer(2);
    Message first("first", "payload-1");
    Message second("second", "payload-2");

    EXPECT_TRUE(buffer.store(first));
    EXPECT_TRUE(buffer.store(second));
    EXPECT_TRUE(buffer.full());

    Message popped;
    ASSERT_TRUE(buffer.pop(popped));
    EXPECT_EQ(first.getId(), popped.getId());
    EXPECT_EQ(first.getHeader(), popped.getHeader());

    ASSERT_TRUE(buffer.pop(popped));
    EXPECT_EQ(second.getId(), popped.getId());
    EXPECT_EQ(second.getPayload(), popped.getPayload());

    EXPECT_FALSE(buffer.pop(popped));
    EXPECT_TRUE(buffer.empty());
}

TEST(BufferTests, DropsWhenFull)
{
    Buffer buffer(1);

    EXPECT_TRUE(buffer.store(Message("accepted")));
    EXPECT_FALSE(buffer.store(Message("dropped")));
}

