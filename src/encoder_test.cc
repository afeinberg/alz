#include "gtest/gtest.h"

#include "encoder.h"
#include "decoder.h"

namespace {

using namespace alz;
using std::make_shared;

static const int kBufLen = 4096;

static const char *kSampleStr =
        "Four score and seven years ago our fathers brought forth on this" \
        "continent, a new nation, conceived in Liberty, and dedicated to the" \
        "proposition that all men are created equal." \
        "Now we are engaged in a great civil war, testing whether that nation," \
        "or any nation so conceived and so dedicated, can long endure. We are" \
        "met on a great battle-field of that war. We have come to dedicate a" \
        "portion of that field, as a final resting place for those who here" \
        "gave their lives that that nation might live. It is altogether fitting" \
        "and proper that we should do this." \
        "But, in a larger sense, we can not dedicate -- we can not consecrate" \
        "-- we can not hallow -- this ground. The brave men, living and dead," \
        "who struggled here, have consecrated it, far above our poor power to" \
        "add or detract. The world will little note, nor long remember what we" \
        "say here, but it can never forget what they did here. It is for us the" \
        "living, rather, to be dedicated here to the unfinished work which they" \
        "who fought here have thus far so nobly advanced. It is rather for us" \
        "to be here dedicated to the great task remaining before us -- that" \
        "from these honored dead we take increased devotion to that cause for" \
        "which they gave the last full measure of devotion -- that we here" \
        "highly resolve that these dead shall not have died in vain -- that" \
        "this nation, under God, shall have a new birth of freedom -- and that" \
        "government of the people, by the people, for the people, shall not" \
        "perish from the earth.";

class EncoderTest : public ::testing::Test {
  protected:

    EncoderTest()
            :encoder_in_(new char[kBufLen]),
             encoder_out_(new char[kBufLen]),
             decoder_out_(new char[kBufLen]) { }

    virtual ~EncoderTest() {
        delete [] encoder_in_;
        delete [] encoder_out_;
        delete [] decoder_out_;
    }

    virtual void SetUp() {
        memset(encoder_in_, 0, kBufLen);
        memset(encoder_out_, 0, kBufLen);
        memset(decoder_out_, 0, kBufLen);
        strcpy(encoder_in_, kSampleStr);
        
        auto encoder_src = make_shared<ByteArraySource>(encoder_in_,
                                                        strlen(encoder_in_));
        auto encoder_sink = make_shared<ByteArraySink>(encoder_out_);
        Encoder enc(encoder_src, encoder_sink);
        enc.encode();
        enc.flush();
        encoded_size_ = enc.written();
    }

    char *encoder_in_;
    char *encoder_out_;
    char *decoder_out_;
    size_t encoded_size_;
};

TEST_F(EncoderTest, test_sample_string) {
    auto decoder_src = make_shared<ByteArraySource>(encoder_out_,
                                                    encoded_size_);
    auto decoder_sink = make_shared<ByteArraySink>(decoder_out_);
    Decoder dec(decoder_src, decoder_sink);
    dec.decode();
    EXPECT_STREQ(kSampleStr, decoder_out_);
}
    
} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
