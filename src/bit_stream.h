// -*-c++-*-

#ifndef ALZ_BIT_STREAM_H_
#define ALZ_BIT_STREAM_H_

#include <cassert>
#include <cstring>

#include <memory>

#include "sink_source.h"

namespace alz {

using std::shared_ptr;

static const int kDefaultBufLen = 8;

class InBitStream {
  public:
    InBitStream(const shared_ptr<Source> &src,
                size_t buf_len = kDefaultBufLen);
    ~InBitStream();

    size_t available() const;
    
    bool next();
  private:
    bool get_bit(size_t n);
    
    shared_ptr<Source> src_;
    size_t buf_len_;
    size_t buf_bits_;
    const char *buf_;
    size_t pos_;
};

inline size_t InBitStream::available() const {
    return buf_bits_ - pos_ + src_->available() * 8;
}

inline bool InBitStream::get_bit(size_t n) {
    assert(n < buf_bits_);
    return (buf_[n / 8] >> (n % 8)) & 1;
}

inline bool InBitStream::next() {
    bool ret;
    if (pos_ == buf_bits_) {
        pos_ = 0;
    }
    if (pos_ == 0) {
        if (src_->available() > 0) {
            buf_ = src_->peek();
            if (src_->available() < buf_len_) {
                src_->skip(src_->available());
            } else {
                src_->skip(buf_len_);
            }
        }
    }
    ret = get_bit(pos_++);
    return ret;
}

class OutBitStream {
  public:
    OutBitStream(const shared_ptr<Sink> &sink,
                 size_t buf_len = kDefaultBufLen);
    ~OutBitStream();

    void append(bool bit);
    void flush();
  private:
    void put_bit(size_t n, bool bit);
    
    shared_ptr<Sink> sink_;
    size_t buf_len_;
    size_t buf_bits_;
    char *buf_;
    size_t pos_;
};

inline void OutBitStream::put_bit(size_t n, bool bit) {
    if (bit) {
        buf_[n / 8] |= 1 << (n % 8);
    } else {
        buf_[n / 8] &= ~(1 << (n % 8));
    }
}

inline void OutBitStream::append(bool bit) {
    put_bit(pos_, bit);
    if (++pos_ == buf_bits_) {
        flush();
    }
}

} // namespace alz

#endif // ALZ_BIT_STREAM_H_
