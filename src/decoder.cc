#include <cstdint>

#include "decoder.h"

namespace alz {

namespace internal {

template <typename IntType, int NBits>
inline IntType read_from_stream(InBitStream *inb) {
    IntType ret = 0;
    for (int i = 0; i < NBits && inb->available() > 0; ++i) {
        ret |= inb->next() << i;
    }
    return ret;
}

} // namespace internal

Decoder::Decoder(const shared_ptr<Source> &src,
                 const shared_ptr<Sink> &sink)
        :src_(src),
         sink_(sink),
         inb_(src_) { }

Decoder::~Decoder() { }

void Decoder::decode() {
    while (inb_.available() > 0) {
        bool compressed = inb_.next();
        if (compressed) {
            uint16_t locn;
            uint8_t len;
            const char *data;
            locn = internal::read_from_stream<uint16_t, 12>(&inb_);
            len = internal::read_from_stream<uint16_t, 4>(&inb_);
            data = sink_->peek_back(locn, len);
            sink_->append(data, len);
        }  else {
            char byte = internal::read_from_stream<char, 8>(&inb_);
            sink_->append(&byte, 1);
        }
    }
}

} // namespace alz
