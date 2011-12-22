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
         fd_(-1),
         left_(buf_len_),
         gpos_(0),
         double_buf_(new char [buf_len_*2]),
         buf_(double_buf_ + buf_len_),
         ptr_(buf_) {
    memset(double_buf_, 0, buf_len_*2);
}

FileSink::~FileSink() {
    delete [] double_buf_;
}

void FileSink::append(const char *bytes, size_t n) {
    if (n <= left_) {
        memcpy(ptr_, bytes, n);
        ptr_ += n;
        left_ -= n;
    } else {
        memcpy(ptr_, bytes, left_);
        n -= left_;
        bytes += left_;
        ptr_ += left_;
        flush();
        memcpy(double_buf_, buf_, buf_len_);
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
    size_t count = ptr_ - buf_;
    size_t nb = write(fd_, buf_, count);
    if (nb < count) {
        perror("write()");
    } else {
        ptr_ = buf_;
        left_ = buf_len_;
    }
}

} // namespace alz
