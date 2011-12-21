#include <cstring>

#include "gtest/gtest.h"

#include "bit_stream.h"

namespace {

using namespace alz;
using std::make_shared;

static const int kDestBufLen = 64;

class BitStreamTest : public ::testing::Test {    
  protected:
    BitStreamTest()
            :str_src_("hello world"),
             str_dest_(new char[kDestBufLen]),
             source_(make_shared<ByteArraySource>(str_src_,
                                                  strlen(str_src_) + 1)),
             sink_(make_shared<ByteArraySink>(str_dest_)),
             inb_(source_),
             outb_(sink_) {
        memset(str_dest_, 0, kDestBufLen);
    }
    
    virtual ~BitStreamTest() {
        delete [] str_dest_;
    }

    const char *str_src_;
    char *str_dest_;
    shared_ptr<ByteArraySource> source_;
    shared_ptr<ByteArraySink> sink_;
    InBitStream inb_;
    OutBitStream outb_;
};

TEST_F(BitStreamTest, test_next_append) {
    while (inb_.available() > 0) {
        outb_.append(inb_.next());
    }
    outb_.flush();
    EXPECT_STREQ(str_src_, str_dest_);
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
