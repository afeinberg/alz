#include "encoder.h"

namespace alz {

Encoder::Encoder(const shared_ptr<Source> &src,
                 const shared_ptr<Sink> &sink)
        :src_(src),
         sink_(sink),
         outb_(sink_),
         init_pos_(sink_->pos()) { }

Encoder::~Encoder() { }

void Encoder::encode() {
    output_byte();
    while (src_->available() > 0) {
        if(src_->available() < kMinLookAhead) {
            output_byte();
            continue;
        }
        bool matched = false;
        bool looking = true;
        size_t look_ahead = kMinLookAhead;
        uint16_t locn = 0;
        uint8_t len = 0;
        while (looking) {
            bool have_match = find_match(src_->peek(),
                                         look_ahead,
                                         &locn,
                                         &len);
            if (!have_match) {
                looking = false;
            } else {
                matched = true;
                if (src_->available() > look_ahead) {
                    look_ahead++;
                } else {
                    looking = false;
                }
            }
        }
        if (!matched) {
            output_byte();
        } else {
            emit_compressed(locn, len);
            src_->skip(len);
        }
    }
}

void Encoder::flush() {
    outb_.flush();
}

} // namespace alz
