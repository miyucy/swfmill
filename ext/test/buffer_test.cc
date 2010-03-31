#include <gtest/gtest.h>
#include "buffer.h"

class BufferTest : public testing::Test
{
};

TEST_F(BufferTest, Constructor)
{
    buffer<char> temp(1);
}

TEST_F(BufferTest, CopyConstructor)
{
    buffer<char> tmp1(5);
    tmp1.append("54321", 5);

    buffer<char> tmp2 = tmp1;
    ASSERT_EQ(0, memcmp("54321", tmp2.ptr(), tmp2.size()));
}

TEST_F(BufferTest, Ptr)
{
    buffer<char> temp(1);
    ASSERT_TRUE(temp.ptr() != NULL);
}

TEST_F(BufferTest, Size)
{
    buffer<char> temp(1);
    ASSERT_EQ(0, temp.size());
}

TEST_F(BufferTest, Capacity)
{
    buffer<char> temp(1);
    ASSERT_EQ(1, temp.capacity());
}

TEST_F(BufferTest, Raise_Capacity)
{
    buffer<char> tmp1(4);

    tmp1.append("12345", 5);
    EXPECT_EQ(5, tmp1.size());
    EXPECT_EQ(11, tmp1.capacity());

    buffer<char> tmp2(2);

    tmp2.append("12345", 5);
    EXPECT_EQ(5, tmp2.size());
    EXPECT_EQ(9, tmp2.capacity());
}

TEST_F(BufferTest, Append)
{
    const char* data = "abc";
    buffer<char> temp;

    ASSERT_EQ(3, temp.append(data, strlen(data)));
    ASSERT_EQ(0, memcmp(data, temp.ptr(), temp.size()));
}

TEST_F(BufferTest, Reserve)
{
    buffer<char> temp(3);
    temp.append("1234", 4);
    ASSERT_EQ(3, temp.reserve(3));
    ASSERT_EQ(3, temp.size());
    ASSERT_EQ(3, temp.capacity());
    ASSERT_EQ(0, memcmp("123", temp.ptr(), temp.size()));
}

TEST_F(BufferTest, Resize)
{
    buffer<char> temp(5);

    temp.append("12345", 5);
    temp.resize(3);
    ASSERT_EQ(0, memcmp("123", temp.ptr(), temp.size()));

    temp.resize(10);
    ASSERT_EQ(12, temp.capacity());
}
