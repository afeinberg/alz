#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file_io.h"

namespace alz {

FileSink::FileSink(const char *path, size_t buf_len)
        :path_(path),
         buf_len_(buf_len),
         mid_point_(buf_len / 2),
         fd_(-1),
         gpos_(0),
         bpos_(0),
         first_flush_(true),
         buf_(new char [buf_len]) { }

FileSink::~FileSink() {
    delete [] buf_;
}

void FileSink::append(const char *bytes, size_t n) {
    // Check if have enough space to write
    if (bpos_ + n < buf_len_) {
        memcpy(buf_ + bpos_, bytes, n);
        bpos_ += n;
        gpos_ += n;
    } else {
        // If we do not, then: first copy up to end of the buffer
        // Then: flush the buffer, copy buffer from mid_point_ to end to
        // begining of the buffer_, then copy the rest from
        // mid_point_ to mid_point_ + n
        size_t left = buf_len_ - bpos_;
        if (left > 0) {
            memcpy(buf_ + bpos_, bytes, left);
            bytes += left;
            n -= left;
            bpos_ += left;
            gpos_ += left;
        }
        flush();
        memcpy(buf_, buf_ + mid_point_, buf_len_ - mid_point_);
        memset(buf_ + mid_point_, 0, buf_len_ - mid_point_);
        append(bytes, n);
    }
}

bool FileSink::open_file() {
    assert(is_closed());
    fd_ = open(path_, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
    if (fd_ == -1) {
        perror("open()");
        return false;
    }
    return true;
}

bool FileSink::close_file() {
    assert(is_open());
    if (close(fd_) != 0) {
        perror("close()");
        return false;
    }
    return true;
}

void FileSink::flush() {
    char *start;
    size_t count;
    if (first_flush_) {
        start = buf_;
        count = bpos_;
        first_flush_ = false;
    } else {
        start = buf_ + mid_point_;
        count = bpos_ - mid_point_;
    }
    size_t nb = write(fd_, start, count);
    if (nb < count) {
        perror("write()");
    } else {
        bpos_ = mid_point_;
    }
}

} // namespace alz
