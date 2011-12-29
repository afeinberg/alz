#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "constants.h"
#include "file_io.h"

namespace alz {

FileSink::FileSink(const char *path, size_t buf_len)
        :path_(path),
         buf_len_(buf_len),
         fd_(-1),
         left_(buf_len_),
         gpos_(0),
         double_buf_(new char [buf_len_ * 2]),
         buf_(double_buf_ + buf_len_),
         ptr_(buf_) {
}

FileSink::~FileSink() {
    delete [] double_buf_;
}

void FileSink::append(const char *bytes, size_t n) {
    if (n <= left_) {
        memcpy(ptr_, bytes, n);
        ptr_ += n;
        left_ -= n;
        gpos_ += n;
    } else {
        memcpy(ptr_, bytes, left_);
        n -= left_;
        bytes += left_;
        ptr_ += left_;
        gpos_ += left_;
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

FileSource::FileSource(const char *path, size_t buf_len)
        :path_(path),
         buf_len_(buf_len),
         fd_(-1),
         gpos_(0),
         gleft_(0),
         bleft_(0),
         double_buf_(new char[buf_len_ * 2]),
         buf_(double_buf_ + buf_len_),
         ptr_(buf_ + buf_len_) {
}

FileSource::~FileSource() {
    delete [] double_buf_;
}

void FileSource::skip(size_t n) {
    assert(n <= constants::kMaxLen);
    assert(is_open());
    ptr_ += n;
    gpos_ += n;
    bleft_ -= n;   
    gleft_ -= n;
    if (bleft_ < constants::kMaxLen) {
        memcpy(double_buf_, buf_, buf_len_);
        fetch_page();        
    }
}

bool FileSource::open_file() {
    assert(is_closed());
    struct stat sb;
    if (stat(path_, &sb) == -1) {
        perror("stat");
        return false;
    }
    gleft_ = sb.st_size;
    fd_ = open(path_, O_RDONLY);
    if (fd_ == -1) {
        perror("open()");
        return false;
    }
    assert(fetch_page());
    return true;    
}

bool FileSource::close_file() {
    assert(is_open());
    if (close(fd_) != 0) {
        perror("close()");
        return false;
    }
    return true;
}
    
bool FileSource::fetch_page() {
    ssize_t nb = read(fd_, buf_, buf_len_);
    if (nb < 0) {
        perror("read()");
        return false;
    } 
    ptr_ -= buf_len_;
    bleft_ += buf_len_;
    return true;
}

} // namespace alz
