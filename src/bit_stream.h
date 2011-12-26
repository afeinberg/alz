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
                size_t buf_len = kDefaultBufLen)
        :src_(src),
         buf_len_(buf_len),
         buf_bits_(buf_len * 8),
         buf_(NULL),
         pos_(0) { }
    
    ~InBitStream() { }

    size_t available() const;
    
    bool next();
    
    template <typename IntType, int NBits>
    IntType next_bits();
    
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

// Borrowed from Bit Vector implementation in
// C Interfaces and Implementations
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
            size_t to_skip = src_->available() < buf_len_ ?
                src_->available() : buf_len_;
            src_->skip(to_skip);
            buf_ = src_->peek() - to_skip;           
        }
    }
    ret = get_bit(pos_++);
    return ret;
}

template <typename IntType, int NBits>
inline IntType InBitStream::next_bits() {
    IntType ret = 0;
    for (int i = 0; i < NBits && available() > 0; ++i) {
        ret |= next() << i;
    }
    return ret;
}

class OutBitStream {
  public:
    OutBitStream(const shared_ptr<Sink> &sink,
                 size_t buf_len = kDefaultBufLen);
    
    ~OutBitStream() { delete [] buf_; }

    void append(bool bit);

    template <typename IntType, int NBits>
    void append_bits(IntType val);
    
    void flush();
  private:
    void put_bit(size_t n, bool bit);
    
    shared_ptr<Sink> sink_;
    size_t buf_len_;
    size_t buf_bits_;
    char *buf_;
    size_t pos_;
};

inline OutBitStream::OutBitStream(const shared_ptr<Sink> &sink,
                                  size_t buf_len)
        :sink_(sink),
         buf_len_(buf_len),
         buf_bits_(buf_len * 8),
         buf_(new char[buf_len]),
         pos_(0) {
    memset(buf_, 0, buf_len);
}

// Borrowed from Bit Vector implementation in
// C Interfaces and Implementations
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

template <typename IntType, int NBits>
inline void OutBitStream::append_bits(IntType val) {
    for (int i = 0; i < NBits; ++i) {
        append((val >> i) & 1);
    }
}

inline void OutBitStream::flush() {
    sink_->append(buf_, buf_len_);
    memset(buf_, 0, buf_len_);
    pos_ = 0;
}


} // namespace alz

#endif // ALZ_BIT_STREAM_H_
