// -*-c++-*-

#ifndef ALZ_ENCODER_H_
#define ALZ_ENCODER_H_

#include <cstdint>
#include <cstring>
#include <cstdio>

#include <vector>
#include <algorithm>

#include <boost/pool/pool.hpp>

#include "util.h"
#include "bit_stream.h"
#include "constants.h"

namespace alz {

using std::shared_ptr;

class Encoder {
  public:
    Encoder(const shared_ptr<Source> &src,
            const shared_ptr<Sink> &sink);
    ~Encoder();

    // How many bytes were written
    size_t written() const { return sink_->pos() - init_pos_; }
    
    // Exposed as public for testing in decoder_test.cc
    void emit_literal(char byte);

    // Exposed as public for testing in decoder_test.cc
    void emit_compressed(uint16_t off, uint8_t len);

    // Encode the source to sink
    void encode();

    // Flush out the rest of the buffer to source
    void flush();    
  private:
    static const size_t kMinLookAhead = 3;
    static const size_t kHashLen = 16384;

    static size_t hash(uint32_t n);    
    static uint32_t get_3bytes(const char *inp);
    
    void output_byte();
    
    shared_ptr<Source> src_;
    shared_ptr<Sink> sink_;    
    OutBitStream outb_;
    size_t init_pos_;
    size_t *hash_tbl_;
};


inline size_t Encoder::hash(uint32_t n)  {  
    return ((n * 57321) >> 9) & (kHashLen - 1);    
}

inline uint32_t Encoder::get_3bytes(const char *inp) {
    uint32_t *ptr = (uint32_t *) inp;
    return *ptr & 0xffffff; // get the first 3 bytes
}

inline void Encoder::emit_literal(char byte) {
    outb_.append(false);
    outb_.append_bits<char, 8>(byte);
}

inline void Encoder::emit_compressed(uint16_t locn, uint8_t len) {
    outb_.append(true);
    outb_.append_bits<uint16_t, 12>(locn);
    outb_.append_bits<uint8_t, 4>(len);
}

inline void Encoder::output_byte() {
    const char *byte = src_->peek();
    emit_literal(*byte);
    src_->skip(1);
}

} // namespace alz

#endif // ALZ_ENCODER_H_
