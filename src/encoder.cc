#include "encoder.h"

namespace alz {

Encoder::Encoder(const shared_ptr<Source> &src,
                 const shared_ptr<Sink> &sink)
        :src_(src),
         sink_(sink),
         outb_(sink_),         
         init_pos_(sink_->pos()),
         hash_tbl_(NULL),
         hash_found_(0),
         hash_not_found_(0) { }

Encoder::~Encoder() {
    printf("hash_found = %d, hash_not_found = %d\n",
           hash_found_,
           hash_not_found_);
    if (hash_tbl_ != NULL) {
        delete [] hash_tbl_;
    }
}

void Encoder::init_hash() {
    hash_tbl_ = new size_t[kHashLen];
    memset(hash_tbl_, 0, kHashLen);
}

void Encoder::encode() {
    init_hash();
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


// NOTE: *** This is slow ***
// The significant way to speed this up is by using a hash table
// and ignoring any hits in the hash table outside of the current
// window.
// 
// However, a naive approach (using std::unordered_map with
// a custom hash and equals functor) had high overhead in this case
// (due to allocations being done under the covers).
//
// Using a "Rabin-Karp"-style approach (see memmem_opt.h) vs. naively
// searching through the window _did_ yield a significant speed up vs.
// the previous naive way (which essentially implemented memmem()) that
// I used

bool Encoder::find_match(const char *inp, size_t look_ahead) {
   
    if (find_in_hash(inp, look_ahead)) {
        hash_found_++;
        return true;
    }
    
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
        hash_not_found_++;
        match_locn_ = lim - pos;        
        match_len_ = look_ahead;
        add_to_hash(inp, match_len_, match_locn_);
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

bool Encoder::find_in_hash(const char *inp, uint8_t len) {
    size_t hash = hash_fn(inp, len);
    size_t val = hash_tbl_[hash];
    if (val == 0) {
        return false;
    }
    size_t try_locn = src_->pos() - val;
    if (try_locn > constants::kMaxOffset) {
        hash_tbl_[hash] = 0;
        return false;
    } else {
        if (memcmp(inp, src_->peek_back(try_locn), len) == 0) {
            match_locn_ = try_locn;
            match_len_ = len;
            //DLOG("found");
            return true;
        }
    }
    return false;
}
    
} // namespace alz
