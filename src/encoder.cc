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

bool Encoder::find_match(const char *inp,
                         size_t look_ahead,
                         uint16_t *locn,
                         uint8_t *len) {
    size_t off;
    size_t lim;
    if (look_ahead > constants::kMaxLen) {
        return false;
    }
    if (src_->pos() - 1 > constants::kMaxOffset) {
        off = constants::kMaxOffset;
        lim = constants::kMaxOffset;
    } else {
        off = src_->pos() - 1;
        lim = src_->pos() - 1;        
    }    
    const char *win = src_->peek_back(off);
    size_t pos;
    if (find_in_window(win, lim, inp, look_ahead, &pos)) {
        *locn = off - pos;
        *len = look_ahead;       
        return true;
    }
    return false;
}
    
bool Encoder::find_in_window(const char *haystack,
                             size_t haystack_len,
                             const char *needle,
                             size_t needle_len,
                             size_t *needle_pos) {
    const char *needle_found;
    needle_found = static_cast<const char *>(memmem_opt((void *) haystack,
                                                        haystack_len,
                                                        (void *) needle,
                                                        needle_len));
    if (needle_found != NULL) {
        *needle_pos = needle_found - haystack;
        return true;
    }
    return false;
}

    
} // namespace alz
