#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include "deflate.h"

class DeflateTest : public testing::Test
{
protected:

    void SetUp()
    {
        read_file("test.dat", testdata, testdata_size);
        testdata_length = 256;

        testbuff = new unsigned char[testdata_length];
        for(size_t i=0; i<testdata_length; i++)
        {
            testbuff[i] = i;
        }
    }

    void TearDown()
    {
        delete[] testdata;
        delete[] testbuff;
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
    unsigned char* testbuff;

};

TEST_F(DeflateTest, Constructor)
{
    Deflate d((unsigned char*)testdata, testdata_size);
}

TEST_F(DeflateTest, compress)
{
    Deflate d(testbuff, testdata_length);
    EXPECT_TRUE(d.compress());
    ASSERT_TRUE(memcmp(d.data(), testdata, testdata_length) == 0);
    ASSERT_EQ(testdata_size, d.size());
}
