// -*-c++-*-

/**
 * Buffered file source and sink
 *
 * Note: since I want to be able to access the buffer, I am not
 * using conventional FILE or ifstream for this, I am sticking with
 * write()/read() system calls and doing my own buffering.
 *
 * Double buffering still occurs with the OS page cache, but I am
 * avoiding the use of direct I/O for now. I am not using memory
 * mapped I/O for this as the accesses are almost entirely sequential.
 */

#ifndef ALZ_FILE_IO_H_
#define ALZ_FILE_IO_H_

#include <cassert>

#include "sink_source.h"

namespace alz {

static const size_t kDefaultFileSinkBufLen = 4096;

class FileSink : public Sink {
  public:
    FileSink(const char *path, size_t buf_len = kDefaultFileSinkBufLen);
    virtual ~FileSink();

    virtual void append(const char *bytes, size_t n);
    virtual const char *peek_back(size_t offset) { return ptr_ - offset - 1; }
    virtual size_t pos() const { return gpos_; }
    
    bool open_file();
    bool close_file();
    void flush();
    
    bool is_open() const { return fd_ != -1; }
    bool is_closed() const { return fd_ == -1; }
  private:
    const char *path_;
    size_t buf_len_;
    int fd_;
    size_t left_; // Space remaining in the buffer
    size_t gpos_;  // Global position
    char *double_buf_;
    char *buf_;
    char *ptr_;
};


/**
class FileSource : public Source {
    explicit FileSource(const char *path);
    virtual ~FileSink();
    
    };
*/

} // namespace alz

#endif // ALZ_FILE_IO_H_