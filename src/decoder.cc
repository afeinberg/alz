#include <cstdint>

#include "decoder.h"

namespace alz {

Decoder::Decoder(const shared_ptr<Source> &src,
                 const shared_ptr<Sink> &sink)
        :src_(src),
         sink_(sink),
         inb_(src) { }

Decoder::~Decoder() { }

void Decoder::decode() {
    while (inb_.available() > 0) {
        bool compressed = inb_.next();
        if (compressed) {
            uint16_t locn = 0;
            uint8_t len = 0;
            const char *data;
            for (int i = 0; i < 12; ++i) {
                locn |= inb_.next() << i;
            }
            for (int i = 0; i < 4; ++i) {
                len |= inb_.next() << i;
            }
            data = sink_->peek_back(locn);
            sink_->append(data, len);
        } else {
            char byte = 0;
            for (int i = 0; i < 8; ++i) {                
                byte |= inb_.next() << i;
            }
            sink_->append(&byte, 1);
        }
    }
}

} // namespace alz
