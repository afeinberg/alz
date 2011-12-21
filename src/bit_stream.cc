#include "bit_stream.h"

namespace alz {

InBitStream::InBitStream(const shared_ptr<Source> &src,
                         size_t buf_len)
        :src_(src),
         buf_len_(buf_len),
         buf_bits_(buf_len * 8),
         buf_(NULL),
         pos_(0) { }

InBitStream::~InBitStream() { }

OutBitStream::OutBitStream(const shared_ptr<Sink> &sink,
                           size_t buf_len)
        :sink_(sink),
         buf_len_(buf_len),
         buf_bits_(buf_len * 8),
         buf_(new char[buf_len]),
         pos_(0) {
    memset(buf_, 0, buf_len);
}

OutBitStream::~OutBitStream() {
    delete [] buf_;
}

void OutBitStream::flush() {
    sink_->append(buf_, buf_len_);
    memset(buf_, 0, buf_len_);
    pos_ = 0;
}

} // namespace alz
