#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include "inflate.h"

class InflateTest : public testing::Test
{
protected:

    void SetUp()
    {
        read_file("test.dat", testdata, testdata_size);
        testdata_length = 256;
    }

    void TearDown()
    {
        delete[] testdata;
    }

    void read_file(const char* const filename, char*& data, size_t& length)
    {
        std::ifstream is;

        is.open(filename, std::ios::binary);

        is.seekg(0, std::ios::end);
        length = is.tellg();
        is.seekg(0, std::ios::beg);

        data = new char[length];
        is.read(data, length);
        is.close();
    }

    char*  testdata;
    size_t testdata_size;
    size_t testdata_length;

};

TEST_F(InflateTest, Constructor)
{
    Inflate i((unsigned char*)testdata, testdata_size, testdata_length);
}

TEST_F(InflateTest, data)
{
    Inflate i((unsigned char*)testdata, testdata_size, testdata_length);
    ASSERT_TRUE(i.data() != NULL);
}

TEST_F(InflateTest, decompress)
{
    Inflate i((unsigned char*)testdata, testdata_size, testdata_length);
    ASSERT_TRUE(i.decompress());
}

TEST_F(InflateTest, decompress_1)
{
    Inflate i((unsigned char*)testdata, testdata_size, testdata_length);
    EXPECT_TRUE(i.decompress());

    const unsigned char f[] = {0,1,2,3,4,5,6,7,8,9};
    ASSERT_TRUE(memcmp(i.data(), f, 10) == 0);
}

TEST_F(InflateTest, decompress_2)
{
    Inflate i(NULL, testdata_size, testdata_length);
    ASSERT_FALSE(i.decompress());
}

TEST_F(InflateTest, decompress_3)
{
    Inflate i((unsigned char*)testdata, testdata_size - 1, testdata_length);
    ASSERT_FALSE(i.decompress());
}

TEST_F(InflateTest, decompress_4)
{
    Inflate i((unsigned char*)testdata, testdata_size, testdata_length - 1);
    ASSERT_FALSE(i.decompress());
}
