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
        matched_ = false;
        bool looking = true;
        size_t look_ahead = kMinLookAhead;
        match_locn_ = 0;
        match_len_ = 0;
        while (looking && look_ahead <= constants::kMaxLen) {
            bool have_match = find_match(src_->peek(), look_ahead);
            if (!have_match) {
                looking = false;
            } else {
                matched_ = true;
                if (src_->available() > look_ahead) {
                    look_ahead++;
                } else {
                    looking = false;
                }
            }
        }
        if (!matched_) {
            output_byte();
        } else {
            emit_compressed(match_locn_, match_len_);
            src_->skip(match_len_);
        }
    }
}

void Encoder::flush() {
    outb_.flush();
    sink_->flush();
}

bool Encoder::find_match(const char *inp, size_t look_ahead) {
    assert(look_ahead <= constants::kMaxLen);
    size_t off;
    size_t lim;
    if (src_->pos() - 1 > constants::kMaxOffset) {
        off = constants::kMaxOffset;
        lim = constants::kMaxOffset;
    } else {
        off = src_->pos() - 1;
        lim = src_->pos() - 1;        
    }
    const char *win;
    if (matched_) {
        win = src_->peek_back(match_locn_);
        lim = match_locn_;
    } else {
        win = src_->peek_back(off);
    }
    size_t pos;
    if (find_in_window(win, lim, inp, look_ahead, &pos)) {
        match_locn_ = lim - pos;        
        match_len_ = look_ahead;       
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
