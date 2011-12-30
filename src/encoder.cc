#include "encoder.h"

namespace alz {

Encoder::Encoder(const shared_ptr<Source> &src,
                 const shared_ptr<Sink> &sink)
        :src_(src),
         sink_(sink),
         outb_(sink_),         
         init_pos_(sink_->pos()),
         hash_tbl_(NULL),
         pool_(sizeof(HashNode)) { }

Encoder::~Encoder() {
#ifdef ALZ_DEBUG_
    printf("added = %ld, found=%ld\n", added_, found_);
#endif // ALZ_DEBUG_
    if (hash_tbl_ != NULL) {
        // Note: there's no need to free the individual
        // lists as the pool allocator takes care of that
        // when its destructor is called.
        delete [] hash_tbl_;
    }
}

void Encoder::init_hash() {
    
#ifdef ALZ_DEBUG_
    added_  = 0;
    found_ = 0;
#endif // ALZ_DEBUG_
    
    hash_tbl_ = new HashNode*[kHashLen];
    memset(hash_tbl_, 0, kHashLen);
}

uint8_t Encoder::find_longest(const char *inp, uint16_t *match_locn) {
    size_t hash = hash_fn(inp);
    HashNode *list = hash_tbl_[hash];
    if (list == NULL) {
        return 0;
    }
    size_t lim = src_->available() < constants::kMaxLen ?
            src_->available() : constants::kMaxLen;
    uint8_t longest = kMinLookAhead;
    uint16_t longest_locn = 0;
    HashNode *node = list;
    HashNode *prev = NULL;
    while (node != NULL && longest < lim) {
        size_t off = src_->pos() - node->pos_;
        if (off > constants::kMaxOffset) {
            if (prev != NULL) {
                prev->next_ = node->next_;
                free_node(node);
                node = prev;
            }
            // don't bother with the case where the first
            // node in the bucket is too far out to keep
            // the code clean
        } else if (off >= longest) {  
            uint8_t len = longest_locn == 0 ? longest : longest + 1;
            bool looking = true;            
            while (looking && len <= off && len <= lim) {
                if (memcmp(inp, src_->peek_back(off), len) == 0) {
                    longest = len;
                    longest_locn = off;
                    len++;
                } else {
                    looking = false;
                }
            }
        }
        prev = node;
        node = node->next_;
    }
    if (longest_locn != 0) {
        
#ifdef ALZ_DEBUG_
        found_++;
#endif // ALZ_DEBUG_
        
        *match_locn = longest_locn;
        return longest;
    }
    return 0;
}
    
void Encoder::encode() {
    init_hash();
    while (src_->available() > 0) {
        if(src_->available() < kMinLookAhead) {
            output_byte();
            continue;
        }
        uint16_t match_locn = 0;
        uint8_t match_len = 0;
        match_len = find_longest(src_->peek(), &match_locn);
        add_to_hash(src_->peek());
        if (match_len == 0) {
            output_byte();            
        } else {
            emit_compressed(match_locn, match_len);
            src_->skip(match_len);
        }
    }
    flush();
}

void Encoder::flush() {
    outb_.flush();
    sink_->flush();
}
   
} // namespace alz
