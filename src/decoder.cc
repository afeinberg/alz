#include <cstdint>

#include "decoder.h"

namespace alz {

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
            locn = inb_.next_bits<uint16_t, 12>();
            len = inb_.next_bits<uint8_t, 4>();
            data = sink_->peek_back(locn);
            sink_->append(data, len);
        }  else {
            char byte = inb_.next_bits<char, 8>();
            sink_->append(&byte, 1);
        }
    }
}

} // namespace alz
