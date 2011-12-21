#include "gtest/gtest.h"

#include "encoder.h"
#include "decoder.h"

namespace {

using namespace alz;
using std::make_shared;

static const int kBufLen = 4096;

class DecoderTest : public ::testing::Test {
  protected:
    DecoderTest()
            :encoder_in_(new char[kBufLen]),
             encoder_out_(new char[kBufLen]),
             decoder_out_(new char[kBufLen]) { }

    virtual ~DecoderTest() {
        delete [] encoder_in_;
        delete [] encoder_out_;
        delete [] decoder_out_;
    }

    virtual void SetUp() {     
        auto encoder_src = make_shared<ByteArraySource>(encoder_in_, kBufLen);
        auto encoder_sink = make_shared<ByteArraySink>(encoder_out_);
        Encoder enc(encoder_src, encoder_sink);
        
        memset(encoder_in_, 0, kBufLen);
        memset(encoder_out_, 0, kBufLen);
        memset(decoder_out_, 0, kBufLen);

        enc.emit_literal('m');
        enc.emit_literal('a');
        enc.emit_literal('h');
        enc.emit_literal('i');
        enc.emit_literal(' ');
        enc.emit_compressed(4, 4);
        enc.flush();
        encoded_size_ = enc.written();
    }

    char *encoder_in_;
    char *encoder_out_;
    char *decoder_out_;
    size_t encoded_size_;
};

TEST_F(DecoderTest, test_decoder) {
    auto decoder_src = make_shared<ByteArraySource>(encoder_out_,
                                                    encoded_size_);
    auto decoder_sink = make_shared<ByteArraySink>(decoder_out_);
    Decoder dec(decoder_src, decoder_sink);
    dec.decode();
    EXPECT_STREQ("mahi mahi", decoder_out_);
}

} // namespace 

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
