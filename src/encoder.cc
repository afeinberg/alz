#include "encoder.h"

namespace alz {

Encoder::Encoder(const shared_ptr<Source> &src,
                 const shared_ptr<Sink> &sink)
        :src_(src),
         sink_(sink),
         outb_(sink_),         
         init_pos_(sink_->pos()),
         hash_tbl_(NULL) {
    hash_tbl_ = (size_t *) calloc(kHashLen, sizeof(size_t));
    assert(hash_tbl_ != NULL);
}

Encoder::~Encoder() {
    if (hash_tbl_ != NULL) {
        free(hash_tbl_);
    }
}

void Encoder::encode() {
    while (src_->available() > 0) {
        if(src_->available() < kMinLookAhead) {
            output_byte();
            continue;
        }
        size_t in_pos = src_->pos();
        const char *inp = src_->peek();
        uint32_t seen = get_3bytes(inp);
        size_t h = hash(get_3bytes(inp));
        size_t ref = hash_tbl_[h];
        hash_tbl_[h] = in_pos + 1;
        
        if (ref == 0) {
            output_byte();
            continue;
        }
        
        size_t off = in_pos - ref;
        const char *ref_ptr = src_->peek_back(off);
        
        if (off > constants::kMaxOffset
            || off <= kMinLookAhead
            || get_3bytes(ref_ptr) != seen) {
            output_byte();
            continue;
        }

        size_t avail = src_->available();
        size_t max_len = avail < constants::kMaxLen ? avail : constants::kMaxLen;
        uint8_t len = kMinLookAhead;
        uint8_t try_len = len;        
        while (try_len <= off && try_len < max_len && ref_ptr[try_len] == inp[try_len]) {
            len = try_len;
            try_len++;
        }
        emit_compressed(off, len);
        src_->skip(len);       
    }
    flush();
}

void Encoder::flush() {
    outb_.flush();
    sink_->flush();
}
   
} // namespace alz
