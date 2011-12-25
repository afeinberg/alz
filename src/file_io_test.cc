#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gtest/gtest.h"

#include "encoder.h"
#include "decoder.h"
#include "file_io.h"

namespace {

using namespace alz;
using std::make_shared;

const char *kPath = "./sample/test_1.txt";

char *slurp(const char *path, size_t *len) {
    struct stat sb;
    ssize_t file_size;
    char *ret;
    int fd;
    if (stat(path, &sb) == -1) {
        perror("stat");
        return NULL;
    }
    *len = file_size = sb.st_size;
    ret = new char[file_size];
    memset(ret, 0, file_size);
    if ((fd = open(path, O_RDONLY)) == -1) {
        perror("open");
        delete [] ret;
        return NULL;
    }
    if ((read(fd, ret, file_size)) != file_size) {
        perror("read");
        delete [] ret;
        return NULL;
    }
    return ret;
}


class FileSinkTest : public ::testing::Test {
  protected:

    FileSinkTest()
            :path_(kPath),
             encoder_in_(NULL),
             encoder_in_size_(0),
             encoder_sink_(make_shared<FileSink>("encoded.dat")),
             decoder_in_(NULL),
             decoder_in_size_(0),
             decoder_sink_(make_shared<FileSink>("decoded.dat")) { }

    virtual ~FileSinkTest() { }
    
    virtual void SetUp() {
        encoder_in_ = slurp(path_, &encoder_in_size_);
        if (encoder_in_ != NULL) {
            encoder_sink_->open_file();
            decoder_sink_->open_file();
        }
    }

    virtual void TearDown() {
        if (encoder_sink_->is_open()) {
            encoder_sink_->close_file();
            unlink("encoded.dat");
        }
        if (decoder_sink_->is_open()) {
            decoder_sink_->close_file();
            unlink("decoded.dat");
        }
        if (encoder_in_ != NULL) {
            delete [] encoder_in_;
        }
        if (decoder_in_ != NULL) {
            delete [] decoder_in_;
        }
    }
    
   

    const char *path_;
    char *encoder_in_;
    size_t encoder_in_size_;
    shared_ptr<FileSink> encoder_sink_;
    char *decoder_in_;
    size_t decoder_in_size_;
    shared_ptr<FileSink> decoder_sink_;    
};


TEST_F(FileSinkTest, test_append_peek_back) {
    auto encoder_src = make_shared<ByteArraySource>(encoder_in_,
                                                    encoder_in_size_);
    Encoder enc(encoder_src, encoder_sink_);
    enc.encode();
    enc.flush();
    encoder_sink_->flush();
    decoder_in_ = slurp("encoded.dat", &decoder_in_size_);
    ASSERT_TRUE(decoder_in_ != NULL);
    auto decoder_src = make_shared<ByteArraySource>(decoder_in_,
                                                    decoder_in_size_);
    Decoder dec(decoder_src, decoder_sink_);
    dec.decode();
    decoder_sink_->flush();
    size_t output_size;
    char *output = slurp("decoded.dat", &output_size);
    ASSERT_TRUE(output != NULL);
    EXPECT_TRUE(memcmp(output, encoder_in_, output_size) == 0);
    delete [] output;
}

class FileSourceTest : public ::testing::Test {
  protected:
    FileSourceTest()
            :path_(kPath),
             encoder_source_(make_shared<FileSource>(path_)),
             encoder_sink_(make_shared<FileSink>("encoded.dat")),
             decoder_source_(make_shared<FileSource>("encoded.dat")),
             decoder_sink_(make_shared<FileSink>("decoded.dat")) { }

    virtual ~FileSourceTest() { }

    // virtual void SetUp() {
    // }

    virtual void TearDown() {
        unlink("encoded.dat");
        unlink("decoded.dat");        
    }

    const char *path_;
    shared_ptr<FileSource> encoder_source_;
    shared_ptr<FileSink> encoder_sink_;
    shared_ptr<FileSource> decoder_source_;
    shared_ptr<FileSink> decoder_sink_;
};

TEST_F(FileSourceTest, test_peek_peekback) {
    encoder_source_->open_file();
    encoder_sink_->open_file();
    Encoder enc(encoder_source_, encoder_sink_);
    enc.encode();
    enc.flush();
    encoder_sink_->flush();
    encoder_source_->close_file();    
    encoder_sink_->close_file();
    
    decoder_source_->open_file();
    decoder_sink_->open_file();
    Decoder dec(decoder_source_, decoder_sink_);
    dec.decode();
    decoder_sink_->flush();
    decoder_sink_->close_file();
    decoder_source_->close_file();
    
    size_t output_size;
    char *expected_output = slurp(path_, &output_size);
    char *actual_output = slurp("decoded.dat", &output_size);
    EXPECT_TRUE(memcmp(expected_output, actual_output, output_size) == 0);
    delete [] expected_output;
    delete [] actual_output;
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
