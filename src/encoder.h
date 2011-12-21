// -*-c++-*-

#ifndef ALZ_ENCODER_H_
#define ALZ_ENCODER_H_

#include <cstdint>

#include "bit_stream.h"

namespace alz {

using std::shared_ptr;
using std::uint8_t;
using std::uint16_t;

class Encoder {
  public:
    Encoder(const shared_ptr<Source> &src,
            const shared_ptr<Sink> &sink);
    ~Encoder();

    void emit_literal(char byte);
    void emit_compressed(uint16_t off, uint8_t len);

    void flush();

    size_t written() const; 
  private:
    static const size_t kMaxLen = 15;
    static const size_t kMaxOffset = 4095;

    shared_ptr<Source> src_;
    shared_ptr<Sink> sink_;
    OutBitStream outb_;
};
    
inline void Encoder::emit_literal(char byte) {
    outb_.append(false);
    for (int i = 0; i < 8; ++i) {
        bool bit = (byte >> i) & 1;
        outb_.append(bit);        
    }    
}

inline void Encoder::emit_compressed(uint16_t locn, uint8_t len) {
    outb_.append(true);
    for (int i = 0; i < 12; ++i) {
        bool bit = (locn >> i) & 1;
        outb_.append(bit);
    }
    for (int i = 0; i < 4; ++i) {
        bool bit = (len >> i) & 1;
        outb_.append(bit);
    }
}

} // namespace alz

#endif // ALZ_ENCODER_H_
