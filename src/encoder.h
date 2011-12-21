// -*-c++-*-

#ifndef ALZ_ENCODER_H_
#define ALZ_ENCODER_H_

#include <cstdint>
#include <cstring>

#include "bit_stream.h"

namespace alz {

namespace internal {

template <typename IntType, int NBits>
inline void write_to_stream(OutBitStream *outb, IntType val) {
    for (int i = 0; i < NBits; ++i) {
        outb->append((val >> i) & 1);
    }
}

// Wrap memmoves
inline bool find_in_window(const char *haystack,
                           size_t haystack_len,
                           const char *needle,
                           size_t needle_len,
                           size_t *needle_pos) {
    const char *needle_found;
    needle_found = static_cast<const char *>(memmem((void *) haystack,
                                                    haystack_len,
                                                    (void *) needle,
                                                    needle_len));
    if (needle_found != NULL) {
        *needle_pos = needle_found - haystack;
        return true;
    }
    return false;
}

} // namespace internal

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
    static const size_t kMaxLen = 15;
    static const size_t kMaxOffset = 4095;
    static const size_t kMinLookAhead = 2;
    
    void output_byte();
    bool find_match(const char *inp,
                    size_t look_ahead,
                    uint16_t *locn,
                    uint8_t *len);
    
    shared_ptr<Source> src_;
    shared_ptr<Sink> sink_;    
    OutBitStream outb_;
    size_t init_pos_;
};
    
inline void Encoder::emit_literal(char byte) {
    outb_.append(false);
    internal::write_to_stream<char, 8>(&outb_, byte);
}

inline void Encoder::emit_compressed(uint16_t locn, uint8_t len) {
    outb_.append(true);
    internal::write_to_stream<uint16_t, 12>(&outb_, locn);
    internal::write_to_stream<uint8_t, 4>(&outb_, len);
}

inline void Encoder::output_byte() {
    const char *byte = src_->peek(1);
    emit_literal(*byte);
    src_->skip(1);
}

inline bool Encoder::find_match(const char *inp,
                                size_t look_ahead,
                                uint16_t *locn,
                                uint8_t *len) {
    size_t off;
    size_t lim;
    if (look_ahead > kMaxLen) {
        return false;
    }
    if (src_->pos() - 1 > kMaxOffset) {
        off = kMaxOffset;
        lim = kMaxOffset;
    } else {
        off = src_->pos() - 1;
        lim = src_->pos() - 1;        
    }
    const char *win = src_->peek_back(off, lim);
    size_t pos;
    if (internal::find_in_window(win, lim, inp, look_ahead, &pos)) {
        *locn = off - pos;
        *len = look_ahead;
        return true;
    }
    return false;
}

} // namespace alz

#endif // ALZ_ENCODER_H_
